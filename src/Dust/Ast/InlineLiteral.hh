<?hh // strict

namespace Dust\Ast;

class InlineLiteral extends InlinePart
{
    /**
     * @var string
     */
    public string $value;

    /**
     * @return string
     */
    public function __toString(): string {
        $str = "";
        
        if (!empty($this->parts))
            foreach ($this->parts as $value) $str .= $value;

        return "\"" . $str . "\"";
    }
}