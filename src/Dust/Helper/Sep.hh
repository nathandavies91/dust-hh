<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class Sep
{
    /**
     * @param \Dust\Evaluate\Chunk $chunk
     * @param \Dust\Evaluate\Context $context
     * @param \Dust\Evaluate\Bodies $bodies
     * @return \Dust\Evaluate\Chunk
     */
    public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies): Evaluate\Chunk {
        $iterationCount = $context->get('$iter');
        if ($iterationCount === NULL)
            $chunk->setError("Sep must be inside array");
        
        $len = $context->get('$len');
        if ($iterationCount < $len - 1)
            return $chunk->render($bodies->block, $context);
        else
            return $chunk;
    }
}