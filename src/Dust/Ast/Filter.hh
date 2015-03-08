<?hh // strict

namespace Dust\Ast;

class Filter extends Ast
{
    /**
     * @var string
     */
    public string $key;

    /**
     * @return string
     */
    public function __toString(): string {
        return "|" . $this->key;
    }
}