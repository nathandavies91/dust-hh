<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class Comparison
{
	/**
	 * @param \Dust\Evaluate\Chunk $chunk
	 * @param \Dust\Evaluate\Context $context
	 * @param \Dust\Evaluate\Bodies $bodies
	 * @param \Dust\Evaluate\Parameters $params
	 * @return \Dust\Evaluate\Chunk
	 */
	public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies, Evaluate\Parameters $params): Evaluate\Chunk {
		// Load value
		if (!isset($params->{"value"}))
			$chunk->setError("Value parameter required");

		$value = $params->{"value"};
		// Load select info
		$selectInfo = $context->get("__selectInfo");
		// Load key
		$key = NULL;
		if (isset($params->{"key"}))
			$key = $params->{"key"};
		else if ($selectInfo != NULL)
			$key = $selectInfo->key;
		else
			$chunk->setError("Must be in select or have key parameter");

		// Check
		if($this->isValid($key, $value))
		{
			if($selectInfo != NULL)
				$selectInfo->selectComparisonSatisfied = true;

			return $chunk->render($bodies->block, $context);
		}
		else if (isset($bodies["else"]))
			return $chunk->render($bodies["else"], $context);
		else
			return $chunk;
	}

	/**
	 * @param string $key
	 * @param string $value
	 * @return bool
	 */
	public function isValid(string $key, string $value): bool {
		return false;
	}
}
