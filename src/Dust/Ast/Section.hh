<?hh // strict

namespace Dust\Ast;

class Section extends Part
{
	/**
	 * @var array[string]
	 */
	public static array<string>
		$acceptableTypes = ["#", "?", "^", "<", "+", "@", "%"],
		$parameters;

	/**
	 * @var string
	 */
	public string $context, $identifier, $type;

	/**
	 * @var Body
	 */
	public Body $body;

	/**
	 * @var array[Body]
	 */
	public array<Body> $bodies;

	/**
	 * @return string
	 */
	public function __toString(): string {
		$str = "{" . $this->type . $this->identifier;

		if ($this->context != NULL)
			$str .= $this->context;

		if (!empty($this->parameters))
			foreach ($this->parameters as $value) $str .= ' ' . $value;

		$str .= "}";

		if ($this->body != NULL)
			$str .= $this->body;

		if (!empty($this->bodies))
			foreach($this->bodies as $value) $str .= $value;

		$str .= "{/" . $this->identifier;

		return $str;
	}
}
