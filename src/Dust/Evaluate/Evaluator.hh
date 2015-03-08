<?hh // strict

namespace Dust\Evaluate;

use Dust\Ast;
use Dust\Dust;
use Dust\Filter;

class Evaluator
{
    /**
     * @var \Dust\Dust
     */
    public Dust $dust;
    
    /**
     * @var \Dust\Evaluate\EvaluatorOptions
     */
    public EvaluatorOptions $options;

    /**
     * @param \Dust\Dust $dust
     * @param \Dust\Evaluate\EvaluatorOptions $options
     */
    public function __construct(Dust $dust, ?EvaluatorOptions $options = NULL): void {
        $this->dust = $dust;
        $this->options = ($options !== NULL) ? $options : new EvaluatorOptions;
    }

    /**
     * @param \Dust\Ast\Ast $ast
     * @param string $message
     * @throws \Dust\Evaluate\EvaluateException
     */
    public function error(?Ast\Ast $ast = NULL, ?string $message = NULL): void {
        throw new EvaluateException($ast, $message);
    }

    /**
     * @param \Dust\Ast\Body $source
     * @param $state
     * @return
     */
    public function evaluate(Ast\Body $source, $state) {
        return trim($this->evaluateBody($source, new Context($this, NULL, new State($state)), new Chunk($this))->getOut());
    }

    /**
     * @param \Dust\Ast\Body $body
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateBody(Ast\Body $body, Context $ctx, Chunk $chunk): Chunk {
        // Go ahead and set the file path on the current context
        if ($body->filePath !== NULL)
            $ctx->currentFilePath = $body->filePath;
        
        foreach ($body->parts as $part)
        {
            if ($part instanceof Ast\Comment) {}
            else if ($part instanceof Ast\Section)
                $chunk = $this->evaluateSection($part, $ctx, $chunk);
            else if ($part instanceof Ast\Partial)
                $chunk = $this->evaluatePartial($part, $ctx, $chunk);
            else if ($part instanceof Ast\Special)
                $chunk = $this->evaluateSpecial($part, $ctx, $chunk);
            else if ($part instanceof Ast\Reference)
                $chunk = $this->evaluateReference($part, $ctx, $chunk);
            else if ($part instanceof Ast\Buffer)
                $chunk = $this->evaluateBuffer($part, $ctx, $chunk);
        }

        return $chunk;
    }
    
    /**
     * @param \Dust\Ast\Buffer $buffer
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateBuffer(Ast\Buffer $buffer, Context $ctx, Chunk $chunk): Chunk {
        $chunk->write($buffer->contents);
        return $chunk;
    }
    
    /**
     * @param \Dust\Ast\Section $section
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateElseBody(Ast\Section $section, Context $ctx, Chunk $chunk): Chunk {
        if ($section->bodies != NULL && count($section->bodies) > 0)
        {
            foreach ($section->bodies as $value)
            {
                if ($value->key == "else")
                    $chunk = $this->evaluateBody($value->body, $ctx, $chunk);
            }
        }

        return $chunk;
    }
    
    /**
     * @param array $params
     * @param \Dust\Evaluate\Context $ctx
     * @return array
     */
    public function evaluateParameters(array $params, Context $ctx): array {
        $ret = [];
        
        foreach ($params as $value)
        {
            if ($value instanceof Ast\NumericParameter)
            {
                if(strpos($value->value, '.') === false)
                    $ret[$value->key] = intval($value->value);
                else
                    $ret[$value->key] = floatval($value->value);
            }
            else if ($value instanceof Ast\IdentifierParameter)
                $ret[$value->key] = $ctx->resolve($value->value);
            else
            {
                // We just set this as the actual AST since it is resolved where it's emitted
                $ret[$value->key] = $value->value;
            }
        }

        return $ret;
    }
    
    /**
     * @param \Dust\Ast\Partial $partial
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluatePartial(Ast\Partial $partial, Context $ctx, Chunk $chunk): Chunk {
        $partialName = $partial->key;
        
        if ($partialName == NULL) $partialName = $this->toDustString($this->normalizeResolved($ctx, $partial->inline, $chunk));
        if ($partialName == NULL) return $chunk;
        
        // Is a named block
        if ($partial->type == "+")
        {
            // Mark beginning
            $chunk->markNamedBlockBegin($partialName);
            // Go ahead and try to replace
            $chunk->replaceNamedBlock($partialName);

            return $chunk;
        }
        
        // Otherwise, we're > ; get base directory
        $basePath = $ctx->currentFilePath;
        if ($basePath != NULL)
            $basePath = dirname($basePath);
        
        // Load partial
        $partialBody = $this->dust->loadTemplate($partialName, $basePath);
        if ($partialBody == NULL)
            return $chunk;
        
        // Null main state
        $state = new State(NULL);
        // Partial context?
        if ($partial->context != NULL)
            $state->forcedParent = $ctx->resolve($partial->context->identifier);
        
        // Params?
        if (!empty($partial->parameters))
            $state->params = $this->evaluateParameters($partial->parameters, $ctx);

        // Render the partial then
        return $this->evaluateBody($partialBody, $ctx->pushState($state), $chunk);
    }
    
    /**
     * @param \Dust\Ast\Reference $ref
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateReference(Ast\Reference $ref, Context $ctx, Chunk $chunk): Chunk {
        // Resolve
        $resolved = $this->normalizeResolved($ctx, $ctx->resolve($ref->identifier), $chunk);

        if (!$this->isEmpty($resolved))
        {
            if ($resolved instanceof Chunk)
                return $resolved;
            
            // Make the string
            if (empty($ref->filters))
            {
                // Fefault filters
                $resolved = array_reduce($this->dust->automaticFilters, function ($prev, Filter\Filter $filter) {
                    return $filter->apply($prev);
                }, $resolved);
            }
            else
            {
                // Apply filters in order...
                $resolved = array_reduce($ref->filters, function ($prev, Filter $curr)
                {
                    if (array_key_exists($curr->key, $this->dust->filters))
                    {
                        $filter = $this->dust->filters[$curr->key];
                        return $filter->apply($prev);
                    }
                    else
                        return $prev;
                }, $resolved);
            }
            $chunk->write($this->toDustString($resolved));
        }

        return $chunk;
    }
    
    /**
     * @param \Dust\Ast\Section $section
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateSection(Ast\Section $section, Context $ctx, Chunk $chunk): Chunk {
        // Stuff that doesn't need resolution
        if ($section->type == "+")
        {
            if($section->identifier->key == NULL)
                $this->error($section->identifier, "Evaluated identifier for partial not supported");
            
            // Mark beginning
            $block = $chunk->markNamedBlockBegin($section->identifier->key);
            // Render default contents
            if ($section->body != NULL)
            {
                $chunk = $this->evaluateBody($section->body, $ctx, $chunk);
                // Mark ending
                $chunk->markNamedBlockEnd($block);
            }
            // Go ahead and try to replace
            $chunk->replaceNamedBlock($section->identifier->key);
        }
        else if ($section->type == "<")
        {
            if ($section->identifier->key == NULL)
                $this->error($section->identifier, "Evaluated identifier for partial not supported");
            
            $chunk->setAndReplaceNamedBlock($section, $ctx);
        }
        else if ($section->type == "@")
        {
            if ($section->identifier->key == NULL)
                $this->error($section->identifier, "Evaluated identifier for helper not supported");
            
            // Do we have the helper?
            if(!isset($this->dust->helpers[$section->identifier->key]))
                $this->error($section->identifier, "Unable to find helper");
            
            $helper = $this->dust->helpers[$section->identifier->key];
            
            // Build state w/ no current value
            $state = new State(NULL);
            
            // Do we have an explicit context?
            if ($section->context != NULL)
                $state->forcedParent = $ctx->resolve($section->context->identifier);
            
            // How about params?
            if (!empty($section->parameters))
                $state->params = $this->evaluateParameters($section->parameters, $ctx);
            
            // Now run the helper
            $chunk = $this->handleCallback($ctx->pushState($state), $helper, $chunk, $section);
        }
        else
        {
            // Build a new state set
            $resolved = $ctx->resolve($section->identifier);
            
            // Build state if not empty
            $state = new State($resolved);
            
            // Do we have an explicit context?
            if($section->context != NULL)
                $state->forcedParent = $ctx->resolve($section->context->identifier);
            
            // How about params?
            if (!empty($section->parameters))
                $state->params = $this->evaluateParameters($section->parameters, $ctx);
            
            // Normalize resolution
            $resolved = $this->normalizeResolved($ctx->pushState($state), $resolved, $chunk, $section);
            
            // Do the needful per type
            switch ($section->type)
            {
                case "#":
                    // Empty means try else
                    if ($this->isEmpty($resolved))
                        $chunk = $this->evaluateElseBody($section, $ctx, $chunk);
                    else if (is_array($resolved) || $resolved instanceof \Traversable)
                    {
                        // Array means loop
                        $iterationCount = -1;
                        foreach ($resolved as $index => $value)
                            $chunk = $this->evaluateBody($section->body, $ctx->push($value, $index, count($resolved), ++$iterationCount), $chunk);
                    }
                    else if ($resolved instanceof Chunk)
                        $chunk = $resolved;
                    else
                    {
                        // Otherwise, just do the body
                        $chunk = $this->evaluateBody($section->body, $ctx->pushState($state), $chunk);
                    }
                    break;
                case "?":
                    // Only if it exists
                    if ($this->exists($resolved))
                        $chunk = $this->evaluateBody($section->body, $ctx, $chunk);
                    else
                        $chunk = $this->evaluateElseBody($section, $ctx, $chunk);
                    break;
                case "^":
                    // Only if it doesn't exist
                    if (!$this->exists($resolved))
                        $chunk = $this->evaluateBody($section->body, $ctx, $chunk);
                    else
                        $chunk = $this->evaluateElseBody($section, $ctx, $chunk);
                    break;
                default:
                    throw new EvaluateException($section, "Unrecognized type: " . $section->type);
            }
        }

        return $chunk;
    }
    
    /**
     * @param \Dust\Ast\Special $spl
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @return \Dust\Evaluate\Chunk
     */
    public function evaluateSpecial(Ast\Special $spl, Context $ctx, Chunk $chunk): Chunk {
        switch ($spl->key)
        {
            case "n":
                $chunk->write("\n");
                break;
            case "r":
                $chunk->write("\r");
                break;
            case "lb":
                $chunk->write("{");
                break;
            case "rb":
                $chunk->write("}");
                break;
            case "s":
                $chunk->write(" ");
                break;
            default:
                throw new EvaluateException($spl, "Unrecognized special: " . $spl->key);
        }

        return $chunk;
    }
    
    /**
     * @param $val
     * @return bool
     */
    public function exists($val): bool {
        // Object exists
        if (is_object($val))
            return true;
        
        // Numeric exists
        if (is_numeric($val))
            return true;
        
        // Empty string does not exist
        if (is_string($val))
            return !empty($val);
        
        // False does not exist
        if (is_bool($val))
            return $val;
        
        // Empty arrays do not exist
        if (is_array($val))
            return !empty($val);
        

        // Nulls do not exist
        return !is_null($val);
    }
    
    /**
     * @param \Dust\Evaluate\Context $ctx
     * @param \Dust\Evaluate\Chunk $chunk
     * @param \Dust\Ast\Section $section
     * @return array
     */
    public function handleCallback(Context $ctx, $callback, Chunk $chunk, ?Ast\Section $section = NULL): array {
        // Reset "this" on closures
        if ($callback instanceof \Closure)
        {
            // Find non-closure new "this"
            $newThis = $ctx->head->value;
            if ($newThis instanceof \Closure)
            {
                // Forced parent?
                if ($ctx->head->forcedParent !== NULL)
                    $newThis = $ctx->head->forcedParent;
                else if ($ctx->parent !== NULL)
                    $newThis = $ctx->parent->head->value;
            }
            // Must be non-closure object
            if (is_object($newThis) && !($newThis instanceof \Closure))
                $callback = \Closure::bind($callback, $newThis);
        }
        
        if (is_object($callback) && method_exists($callback, '__invoke'))
            $reflected = new \ReflectionMethod($callback, '__invoke');
        else
            $reflected = new \ReflectionFunction($callback);
        
        $paramCount = $reflected->getNumberOfParameters();
        $args = [];
        if ($paramCount > 0)
        {
            $args[] = $chunk;
            if ($paramCount > 1)
            {
                $args[] = $ctx;
                if ($paramCount > 2 && $section != NULL)
                {
                    $args[] = new Bodies($section);
                    if ($paramCount > 3)
                        $args[] = new Parameters($this, $ctx);
                }
            }
        }

        // Invoke
        return call_user_func_array($callback, $args);
    }
    
    /**
     * @param $val
     * @return bool
     */
    public function isEmpty($val): bool {
        // Numeric not empty
        if (is_numeric($val))
            return false;

        // Otherwise, normal empty check
        return empty($val);
    }
    
    /**
     * @param \Dust\Evaluate\Context $ctx
     * @param string $resolved
     * @param \Dust\Evaluate\Chunk $chunk
     * @param \Dust\Evaluate\Section $section
     * @return string
     */
    public function normalizeResolved(Context $ctx, ?string $resolved, Chunk $chunk, ?Ast\Section $section = NULL): ?string {
        $handledSpecial = true;
        while ($handledSpecial)
        {
            if (is_callable($resolved) && !is_string($resolved))
                $resolved = $this->handleCallback($ctx, $resolved, $chunk, $section);
            else if ($resolved instanceof Ast\Inline)
            {
                // Resolve full inline parameter
                $newChunk = $chunk->newChild();
                foreach ($resolved->parts as $value)
                {
                    if ($value instanceof Ast\Reference)
                        $newChunk = $this->evaluateReference($value, $ctx, $newChunk);
                    else if ($value instanceof Ast\Special)
                        $newChunk = $this->evaluateSpecial($value, $ctx, $newChunk);
                    else
                        $newChunk->write(strval($value));
                }
                
                $resolved = $newChunk->getOut();
                break;
            }
            else
                $handledSpecial = false;
        }

        return $resolved;
    }

    /**
     * @param $val
     * @return bool
     */
    public function toDustString($val): string {
        if (is_bool($val))
            return $val ? "true" : "false";
        
        if (is_array($val))
            return implode(",", $val);
        
        if (is_object($val) && !method_exists($val, "__toString"))
            return get_class($val);

        return (string) $val;
    }
}