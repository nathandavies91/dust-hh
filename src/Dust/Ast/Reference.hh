<?hh // strict

namespace Dust\Ast;

class Reference extends InlinePart
{
    /**
     * @var array[string]
     */
    public array<string> $filters;
    
    /**
     * @var string
     */
    public string $identifier;

    /**
     * @return string
     */
    public function __toString(): string {
        $str = $this->identifier;
        
        if (!empty($this->filters))
            foreach ($this->filters as $value) $str .= $value;

        return "{" . $str . "}";
    }
}