<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class Size
{
    /**
     * @param \Dust\Evaluate\Chunk $chunk
     * @param \Dust\Evaluate\Context $context
     * @param \Dust\Evaluate\Bodies $bodies
     * @param \Dust\Evaluate\Parameters $params
     * @return \Dust\Evaluate\Chunk
     */
    public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies, Evaluate\Parameters $params): Evaluate\Chunk {
        if (!isset($params->{"key"}))
            $chunk->setError("Parameter required: key");
        
        $key = $params->{"key"};
        if (is_null($key))
            return $chunk->write("0");
        
        if (is_numeric($key))
            return $chunk->write($key);
        
        if (is_string($key))
            return $chunk->write(strlen($key));
        
        if (is_array($key) || $key instanceof \Countable)
            return $chunk->write(count($key));
        
        if (is_object($key))
            return $chunk->write(count(get_object_vars($key)));

        return $chunk->write(strlen(strval($key)));
    }
}