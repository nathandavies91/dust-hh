<?hh // strict

namespace Dust\Evaluate;

use Dust\Ast\Identifier;

class Context
{
	/**
	 * @var \Dust\Evaluate\Evaluator
	 */
	public Evaluator $evaluator;

	/**
	 * @var \Dust\Evaluate\Context
	 */
	public Context $parent;

	/**
	 * @var \Dust\Evaluate\State
	 */
	public State $head;

	/**
	 * @var string
	 */
	public string $currentFilePath;

	/**
	 * @constructor
	 * @param \Dust\Evaluate\Evaluator $evaluator
	 * @param \Dust\Evaluate\Context $parent
	 * @param \Dust\Evaluate\State $head
	 */
	public function __construct(Evaluator $evaluator, ?Context $parent = NULL, ?State $head = NULL): void {
		$this->evaluator = $evaluator;
		$this->parent = $parent;
		$this->head = $head;

		if ($parent != NULL)
			$this->currentFilePath = $parent->currentFilePath;
	}

	/**
	 * @return string
	 */
	public function current(): string {
		if($this->head->forcedParent != NULL)
			return $this->head->forcedParent;

		return $this->head->value;
	}

	/**
	 * @param string $key
	 * @param $value
	 * @return mixed
	 */
	public function findInArrayAccess(string $key, $value) {
		if((is_array($value) || $value instanceof \ArrayAccess) && isset($value[$key]))
			return $value[$key];
		else
			return NULL;
	}

	/**
	 * @param string $key
	 * @param $parent
	 */
	public function findInObject(string $key, $parent) {
		if (is_object($parent) && !is_numeric($key))
		{
			// Prop or method
			if (array_key_exists($key, $parent))
				return $parent->{$key};
			else if (method_exists($parent, $key))
				return (new \ReflectionMethod($parent, $key))->getClosure($parent);
			else if (is_callable([
				$parent,
				"get" . ucfirst($key)
			]))
			{
				$getter = "get" . ucfirst($key);
				return $parent->$getter();
			}
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * @param string $str
	 */
	public function get(string $str) {
		$ident = new Identifier(-1);
		$ident->key = $str;
		$resolved = $this->resolve($ident);
		$resolved = $this->evaluator->normalizeResolved($this, $resolved, new Chunk($this->evaluator));

		if ($resolved instanceof Chunk)
			return $resolved->getOut();

		return $resolved;
	}

	/**
	 * @param var $head
	 * @param var $index
	 * @param int $length
	 * @param int iterationCount
	 * @return \Dust\Evaluate\Context
	 */
	public function push($head, $index = NULL, ?int $length = NULL, ?int $iterationCount = NULL): Context {
		$state = new State($head);

		if ($index !== NULL)
			$state->params["$idx"] = $index;

		if ($length !== NULL)
			$state->params["$len"] = $length;

		if ($iterationCount !== NULL)
			$state->params["$iter"] = $iterationCount;

		return $this->pushState($state);
	}

	/**
	 * @param \Dust\Evaluate\State $head
	 * @return \Dust\Evaluate\Context
	 */
	public function pushState(State $head): Context {
		return new Context($this->evaluator, $this, $head);
	}

	/**
	 * @param string $head
	 * @return \Dust\Evaluate\Context
	 */
	public function rebase(string $head): Context {
		return $this->rebaseState(new State($head));
	}

	/**
	 * @param \Dust\Evaluate\State $head
	 * @return \Dust\Evaluate\Context
	 */
	public function rebaseState(State $head): Context {
		// Gotta get top parent
		$topParent = $this;
		while ($topParent->parent != NULL)
			$topParent = $topParent->parent;

		// Now create
		return new Context($this->evaluator, $topParent, $head);
	}

	/**
	 * @param \Dust\Ast\Identifier $identifier
	 * @param bool $forceArrayLookup
	 * @param string $mainValue
	 */
	public function resolve(Identifier $identifier, $forceArrayLookup = false, ?string $mainValue = NULL) {
		if ($mainValue === NULL)
			$mainValue = $this->head->value;

		// Try local
		$resolved = $this->resolveLocal($identifier, $mainValue, $forceArrayLookup);

		// Forced local?
		if ($identifier->preDot)
			return $resolved;

		// If it's not there, we can try the forced parent
		if ($resolved === NULL && $this->head->forcedParent)
			$resolved = $this->resolveLocal($identifier, $this->head->forcedParent, $forceArrayLookup);

		// If it's still not there, we can try parameters
		if ($resolved === NULL && count($this->head->params) > 0)
			$resolved = $this->resolveLocal($identifier, $this->head->params, true);

		// Not there and not forced parent? walk up
		if($resolved === NULL && $this->head->forcedParent === NULL && $this->parent != NULL)
			$resolved = $this->parent->resolve($identifier, $forceArrayLookup);

		return $resolved;
	}

	/**
	 * @param \Dust\Ast\Identifier $identifier
	 * @param $parantObject
	 * @param bool $forceArrayLookup
	 */
	public function resolveLocal(Identifier $identifier, $parentObject, $forceArrayLookup = false) {
		$key = NULL;

		if ($identifier->key != NULL)
			$key = $identifier->key;
		else if ($identifier->number != NULL)
		{
			$key = intval($identifier->number);

			// If this isn't an array lookup, just return the number
			if (!$forceArrayLookup)
				return $key;
		}

		$result = NULL;

		// No key, no array, but predot means result is just the parent
		if ($key === NULL && $identifier->preDot && $identifier->arrayAccess == NULL)
			$result = $parentObject;

		// Try to find on object (if we aren't forcing array lookup)
		if (!$forceArrayLookup && $key !== NULL)
			$result = $this->findInObject($key, $parentObject);

		// Now, try to find in array
		if ($result === NULL && $key !== NULL)
			$result = $this->findInArrayAccess($key, $parentObject);

		// If it's there (or has predot) and has array access, try to get array child
		if ($identifier->arrayAccess != NULL)
		{
			// Find the key
			$arrayKey = $this->resolve($identifier->arrayAccess, false, $parentObject);
			if ($arrayKey !== NULL)
			{
				$keyIdent = new Identifier(-1);
				if (is_numeric($arrayKey))
					$keyIdent->number = strval($arrayKey);
				else
					$keyIdent->key = (string) $arrayKey;

				// Lookup by array key
				if ($result !== NULL)
					$result = $this->resolveLocal($keyIdent, $result, true);
				else if ($identifier->preDot)
					$result = $this->resolveLocal($keyIdent, $parentObject, true);
			}
		}

		// If it's there and has next, use it
		if ($result !== NULL && $identifier->next && !is_callable($result))
			$result = $this->resolveLocal($identifier->next, $result);

		return $result;
	}
}
