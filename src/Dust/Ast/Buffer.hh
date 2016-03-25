<?hh // strict

namespace Dust\Ast;

class Buffer extends Part
{
	/**
	 * @var string
	 */
	public string $contents;

	/**
	 * @return string
	 */
	public function __toString(): string {
		return $this->contents;
	}
}
