<?hh // strict

namespace Dust\Ast;

class InlineParameter extends Parameter
{
    /**
     * @var string
     */
    public string $value;
    
    /**
     * @return string
     */
    public function __toString(): string {
        return $this->key . "=" . $this->value;
    }
}