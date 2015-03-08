<?hh // strict

namespace Dust\Ast;

class Body extends Ast
{
    /**
     * @var string
     */
    public string $filePath;

    /**
     * @var array[string]
     */
    public array<string> $parts;

    /**
     * @return string
     */
    public function __toString(): string {
        $str = "";
        
        if (!empty($this->parts))
            foreach ($this->parts as $value) $str .= $value;

        return $str;
    }
}