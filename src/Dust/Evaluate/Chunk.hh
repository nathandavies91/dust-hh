<?hh // strict

namespace Dust\Evaluate;

use Dust\Ast\Body;
use Dust\Ast\Section;

class Chunk
{
    /**
     * @var \Dust\Evaluate\Evaluator
     */
    protected Evaluator $evaluator;

    /**
     * @var string
     */
    protected string $out = "";

    /**
     * @var array
     */
    public array
        $pendingNamedBlocks,
        $setNamedStrings,
        $tapStack;
    
    /**
     * @var int
     */
    public int $pendingNamedBlockOffset = 0;

    /**
     * @param \Dust\Evaluate\Evaluator $evaluator
     */
    public function __construct(Evaluator $evaluator): void {
        $this->evaluator = $evaluator;
        $this->pendingNamedBlocks = [];
        $this->setNamedStrings = [];
    }
    
    /**
     * @return string
     */
    public function getOut(): string {
        return $this->out;
    }
    
    /**
     * @param string $name
     */
    public function markNamedBlockBegin($name) {
        if (!array_key_exists($name, $this->pendingNamedBlocks))
            $this->pendingNamedBlocks[ $name ] = [];
        
        $block = (object) [
            "begin" => strlen($this->out),
            "end"   => NULL
        ];
        
        $this->pendingNamedBlocks[$name][] = $block;

        return $block;
    }
    
    /**
     * @param $block
     */
    public function markNamedBlockEnd($block): void {
        $block->end = strlen($this->out);
    }
    
    /**
     * @return \Dust\Evaluate\Chunk
     */
    public function newChild(): Chunk {
        $chunk = new Chunk($this->evaluator);
        $chunk->tapStack = &$this->tapStack;
        $chunk->pendingNamedBlocks = &$this->pendingNamedBlocks;

        return $chunk;
    }
    
    /**
     * @param \Dust\Ast\Body $ast
     * @param \Dust\Evaluate\Context $context
     * @return string
     */
    public function render(Body $ast, Context $context): Chunk {
        $text = $this;
        
        if ($ast != NULL)
        {
            $text = $this->evaluator->evaluateBody($ast, $context, $this);
            if ($this->tapStack != NULL)
                foreach ($this->tapStack as $value) $text->out = $value($text->out);
        }
        
        return $text;
    }
    
    /**
     * @param string $name
     */
    public function replaceNamedBlock(string $name): void {
        // We need to replace inside of chunk the begin/end
        if (array_key_exists($name, $this->pendingNamedBlocks) && array_key_exists($name, $this->setNamedStrings))
        {
            $namedString = $this->setNamedStrings[$name];
            
            // Get all blocks
            $blocks = $this->pendingNamedBlocks[$name];
            
            // We need to reverse the order to replace backwards first to keep line counts right
            usort($blocks, function ($a, $b) {
                return $a->begin > $b->begin ? -1 : 1;
            });
            
            // Hold on to pre-count
            $preCount = strlen($this->out);
            
            // Loop and splice string
            foreach($blocks as $value)
            {
                $text = substr($this->out, 0, $value->begin + $this->pendingNamedBlockOffset) . $namedString;
                
                if ($value->end != NULL)
                    $text .= substr($this->out, $value->end + $this->pendingNamedBlockOffset);
                else
                    $text .= substr($this->out, $value->begin + $this->pendingNamedBlockOffset);
                
                $this->out = $text;
            }
            
            // Now we have to update all the pending offset
            $this->pendingNamedBlockOffset += strlen($this->out) - $preCount;
        }
    }
    
    /**
     * @param \Dust\Ast\Section $section
     * @param \Dust\Evaluate\Context $ctx
     */
    public function setAndReplaceNamedBlock(Section $section, Context $ctx): void {
        $output = "";
        // If it has no body, we don't do anything
        if ($section != NULL && $section->body != NULL)
            $output = $this->evaluator->evaluateBody($section->body, $ctx, $this->newChild())->out;
        
        // Save it
        $this->setNamedStrings[$section->identifier->key] = $output;
        
        // Try and replace
        $this->replaceNamedBlock($section->identifier->key);
    }
    
    /**
     * @param string $error
     * @param \Dust\Ast\Body $ast
     * @return \Dust\Evaluate\Chunk;
     */
    public function setError($error, ?Body $ast = NULL): Chunk {
        $this->evaluator->error($ast, $error);
        return $this;
    }
    
    /**
     * @param callable $callback
     * @return \Dust\Evaluate\Chunk;
     */
    public function tap(callable $callback): Chunk {
        $this->tapStack[] = $callback;

        return $this;
    }
    
    /**
     * @return \Dust\Evaluate\Chunk;
     */
    public function untap(): Chunk {
        array_pop($this->tapStack);

        return $this;
    }

    /**
     * @param string $str
     * @return $this
     */
    public function write(string $str): Chunk {
        $this->out .= $str;
        return $this;
    }
}