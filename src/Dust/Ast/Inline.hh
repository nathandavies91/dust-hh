<?hh // strict

namespace Dust\Ast;

class Inline extends Ast
{
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

		return "\"" . $str . "\"";
	}
}
