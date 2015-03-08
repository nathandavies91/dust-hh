<?hh // strict

namespace Dust\Ast;

class Partial extends Part
{
    /**
     * @var array[string]
     */
    public array<string> $parameters;
    
    /**
     * @var string
     */
    public string $context, $inline, $key, $type;
    
    /**
     * @return string
     */
    public function __toString(): string {
        $str = $this->type;
        
        if ($this->key != NULL)
            $str .= $this->key;
        else
            $str .= $this->inline;
        
        if ($this->context != NULL)
            $str .= $this->context;
        
        if (!empty($this->parameters))
            foreach ($this->parameters as $value) $str .= ' ' . $value;

        return "{" . $str . "/}";
    }

}