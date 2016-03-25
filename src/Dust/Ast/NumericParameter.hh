<?hh // strict

namespace Dust\Ast;

class NumericParameter extends Parameter
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
