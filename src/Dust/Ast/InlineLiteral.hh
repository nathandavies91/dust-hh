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
		return $this->value;
	}
}
