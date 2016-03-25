<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class IfHelper
{
	/**
	 * @param \Dust\Evaluate\Chunk $chunk
	 * @param \Dust\Evaluate\Context $context
	 * @param \Dust\Evaluate\Bodies $bodies
	 * @return \Dust\Evaluate\Chunk
	 */
	public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies): Evaluate\Chunk {
		// Scary and dumb! won't include in default...
		$cond = $context->get("cond");

		if ($cond === NULL)
			$chunk->setError("Unable to find cond for if");
		if (eval("return " . $cond . ";"))
			return $chunk->render($bodies->block, $context);
		else if (isset($bodies["else"]))
			return $chunk->render($bodies["else"], $context);
		else
			return $chunk;
	}
}
