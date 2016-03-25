<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class DefaultHelper
{
	/**
	 * @param \Dust\Evaluate\Chunk $chunk
	 * @param \Dust\Evaluate\Context $context
	 * @param \Dust\Evaluate\Bodies $bodies
	 * @return \Dust\Evaluate\Chunk
	 */
	public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies): Evaluate\Chunk {
		$selectInfo = $context->get("__selectInfo");
		if ($selectInfo == NULL)
			$chunk->setError("Default must be inside select");

		//check
		if (!$selectInfo->selectComparisonSatisfied)
			return $chunk->render($bodies->block, $context);
		else
			return $chunk;
	}
}
