<?hh // strict

namespace Dust\Evaluate;

class State
{
    /**
     * @var
     */
    public $value;

    /**
     * @var bool
     */
    public $forcedParent;

    /**
     * @var array
     */
    public $params;

    /**
     * @param $value
     */
    public function __construct($value): void {
        $this->value = $value;
        $this->params = [];
    }
}