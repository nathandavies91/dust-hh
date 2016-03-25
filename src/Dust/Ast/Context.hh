<?hh // strict

namespace Dust\Ast;

class Context extends Ast
{
	/**
	 * @var string
	 */
	public string $identifier;

	/**
	 * @return string
	 */
	public function __toString(): string {
		return ":" . $this->identifier;
	}
}
