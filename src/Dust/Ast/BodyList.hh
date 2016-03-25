<?hh // strict

namespace Dust\Ast;

class BodyList extends Ast
{
	/**
	 * @var string
	 */
	public string $body, $key;

	/**
	 * @return string
	 */
	public function __toString(): string {
		$str = "{:" . $this->key . "}";
		if ($this->body != NULL)
			$str .= $this->body;

		return $str;
	}
}
