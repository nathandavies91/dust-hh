<?hh // strict

namespace Dust\Ast;

class Identifier extends Ast
{
	/**
	 * @var bool
	 */
	public bool $preDot = false;

	/**
	 * @var string
	 */
	public string $arrayAccess, $key, $next;

	/**
	 * @var int
	 */
	public int $number;

	/**
	 * @return string
	 */
	public function __toString(): string {
		$str = '';

		if ($this->preDot)
			$str .= '.';

		if ($this->key != NULL)
			$str .= $this->key;
		else if ($this->number != NULL)
			$str .= $this->number;

		if ($this->arrayAccess != NULL)
			$str .= "[" . $this->arrayAccess . "]";

		if ($this->next != NULL)
			$str .= $this->next;

		return $str;
	}
}
