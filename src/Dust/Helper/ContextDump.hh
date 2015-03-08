<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class ContextDump
{
    /**
     * @param \Dust\Evaluate\Chunk $chunk
     * @param \Dust\Evaluate\Context $context
     * @param \Dust\Evaluate\Bodies $bodies
     * @param \Dust\Evaluate\Parameters $params
     * @return \Dust\Evaluate\Chunk
     */
    public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies, Evaluate\Parameters $params): Evaluate\Chunk {
        // Get config
        $current = !isset($params->{'key'}) || $params->{'key'} != 'full';
        $output = !isset($params->{'to'}) || $params->{'to'} != 'console';
        
        // Ok, basically we're gonna give parent object w/ two extra values, "__forcedParent__", "__child__", and "__params__"
        $getContext = function (Evaluate\Context $ctx) use ($current, &$getContext)
        {
            // First parent
            $parent = !$current && $ctx->parent != NULL ? $getContext($ctx->parent) : (object) [];
            // Now start adding pieces
            $parent->__child__ = $ctx->head == NULL ? NULL : $ctx->head->value;
            if ($ctx->head != NULL && $ctx->head->forcedParent !== NULL)
                $parent->__forcedParent__ = $ctx->head->forcedParent;
            if ($ctx->head != NULL && !empty($ctx->head->params))
                $parent->__params__ = $ctx->head->params;

            return $parent;
        };
        
        // Now json_encode
        $str = $context->parent == NULL ? '{ }' : json_encode($getContext($context->parent), JSON_PRETTY_PRINT);
        // Now put where necessary
        if ($output)
            return $chunk->write($str);
        
        echo($str . "\n");

        return $chunk;
    }
}