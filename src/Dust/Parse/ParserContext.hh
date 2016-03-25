<?hh // strict

namespace Dust\Parse;

class ParserContext
{
	/**
	 * @var string
	 */
	public $str;

	/**
	 * @var int
	 */
	public $offset = 0;

	/**
	 * @var array
	 */
	public $offsetTransactionStack = [];

	/**
	 * @param string $str
	 */
	public function __construct($str): void {
		$this->str = $str;
	}

	public function beginTransaction(): void {
		$this->offsetTransactionStack[] = $this->offset;
	}

	public function rollbackTransaction(): void {
		$this->offset = array_pop($this->offsetTransactionStack);
	}

	public function commitTransaction(): void {
		array_pop($this->offsetTransactionStack);
	}

	/**
	 * @return object
	 */
	public function getCurrentLineAndCol() {
		return $this->getLineAndColFromOffset($this->offset);
	}

	/**
	 * @param int $offset
	 * @return object
	 */
	public function getLineAndColFromOffset(int $offset) {
		$line = 0;
		$prev = -1;
		while (true)
		{
			$newPrev = strpos($this->str, "\n", $prev + 1);
			if ($newPrev === false || $newPrev > $offset)
				break;

			$prev = $newPrev;
			$line++;
		}
		if ($prev == -1)
			$prev = 0;

		return (object) ["line" => $line + 1, "col" => $offset - $prev];
	}

	/**
	 * @param int $offset
	 * @return string
	 */
	public function peek(int $offset = 1): ?string {
		if (strlen($this->str) <= $this->offset + ($offset - 1))
			return NULL;

		return $this->str[$this->offset + ($offset - 1)];
	}

	/**
	 * @return string
	 */
	public function next(): string {
		$this->offset++;
		if (strlen($this->str) <= ($this->offset - 1))
			return NULL;

		return $this->str[($this->offset - 1)];
	}

	/**
	 * @return bool
	 */
	public function skipWhitespace(): bool {
		$found = false;
		while (in_array($this->peek(), [" ", "\t", "\v", "\f", "\r", "\n"]))
		{
			$this->offset++;
			$found = true;
		}

		return $found;
	}
}
