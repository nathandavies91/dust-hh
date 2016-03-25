<?hh // strict

namespace Dust\Evaluate;

use Dust\Ast\Body;
use Dust\Ast\Section;

class Bodies implements \ArrayAccess
{
	/**
	 * @var \Dust\Ast\Body
	 */
	public Body $block;

	/**
	 * @var \Dust\Ast\Section
	 */
	private Section $section;

	/**
	 * @param \Dust\Ast\Section $section
	 */
	public function __construct(Section $section): void {
		$this->section = $section;
		$this->block = $section->body;
	}

	/**
	 * @param mixed $offset
	 * @return bool
	 */
	public function offsetExists($offset): bool {
		return $this[$offset] != NULL;
	}

	/**
	 * @param mixed $offset
	 * @return \Dust\Ast\Body|null
	 */
	public function offsetGet($offset): ?Body {
		for($i = 0; $i < count($this->section->bodies); $i++)
		{
			if($this->section->bodies[$i]->key == $offset)
				return $this->section->bodies[$i]->body;
		}

		return NULL;
	}

	/**
	 * @param mixed $offset
	 * @param mixed $value
	 * @throws \Dust\Evaluate\EvaluateException
	 */
	public function offsetSet($offset, $value) {
		throw new EvaluateException($this->section, "Unsupported set on bodies");
	}

	/**
	 * @param mixed $offset
	 * @throws \Dust\Evaluate\EvaluateException
	 */
	public function offsetUnset($offset) {
		throw new EvaluateException($this->section, "Unsupported unset on bodies");
	}
}
