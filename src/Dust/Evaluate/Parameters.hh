<?hh // strict

namespace Dust\Evaluate;

class Parameters
{
    /**
     * @var \Dust\Evaluate\Evaluator
     */
    private Evaluator $evaluator;

    /**
     * @var \Dust\Evaluate\Context
     */
    private Context $ctx;

    /**
     * @param \Dust\Evaluate\Evaluator $evaluator
     * @param \Dust\Evaluate\Context $ctx
     */
    public function __construct(Evaluator $evaluator, Context $ctx) {
        $this->evaluator = $evaluator;
        $this->ctx = $ctx;
    }
    
    /**
     * @param string $name
     * @param string
     */
    public function __get(string $name): string {
        if (isset($this->ctx->head->params[$name]))
        {
            $resolved = $this->ctx->head->params[$name];
            $newChunk = new Chunk($this->evaluator);
            $resolved = $this->evaluator->normalizeResolved($this->ctx, $resolved, $newChunk);
            
            if($resolved instanceof Chunk)
                return $resolved->getOut();

            return $resolved;
        }

        return NULL;
    }

    /**
     * @param string $name
     * @return bool
     */
    public function __isset(string $name): bool {
        return isset($this->ctx->head->params) && array_key_exists($name, $this->ctx->head->params);
    }
}