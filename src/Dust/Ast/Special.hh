<?hh // strict

namespace Dust\Ast;

class Special extends InlinePart
{
    /**
     * @var string
     */
    public string $key;

    /**
     * @return string
     */
    public function __toString(): string {
        return "{~" . $this->key . "}";
    }
}