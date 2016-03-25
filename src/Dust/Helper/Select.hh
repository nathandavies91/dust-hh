<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class Select
{
	/**
	 * @param \Dust\Evaluate\Chunk $chunk
	 * @param \Dust\Evaluate\Context $context
	 * @param \Dust\Evaluate\Bodies $bodies
	 * @param \Dust\Evaluate\Parameters $params
	 * @return \Dust\Evaluate\Chunk
	 */
	public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies, Evaluate\Parameters $params): Evaluate\Chunk {
		// Evaluate key here
		if (!isset($params->{"key"}))
			$chunk->setError("Key parameter required");

		$key = $params->{"key"};

		// Just eval body with some special state
		return $chunk->render($bodies->block, $context->pushState(new Evaluate\State((object) ["__selectInfo" => (object) ["selectComparisonSatisfied" => false, "key" => $key]])));
	}
}
