<?hh // strict

namespace Dust\Ast;

class Ast
{
    /**
     * @var int
     */
    public int $offset;

    /**
     * @param $offset
     * @constructor
     */
    public function __construct(int $offset) {
        $this->offset = $offset;
    }
}