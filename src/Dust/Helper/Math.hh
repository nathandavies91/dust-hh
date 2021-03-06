<?hh // strict

namespace Dust\Helper;

use Dust\Evaluate;

class Math
{
	/**
	 * @param \Dust\Evaluate\Chunk $chunk
	 * @param \Dust\Evaluate\Context $context
	 * @param \Dust\Evaluate\Bodies $bodies
	 * @return \Dust\Evaluate\Chunk
	 */
	public function __invoke(Evaluate\Chunk $chunk, Evaluate\Context $context, Evaluate\Bodies $bodies): Evaluate\Chunk {
		$result = $context->get("key");

		if ($result === NULL)
			$chunk->setError("Key required");

		$method = $context->get("method");
		if ($method === NULL)
			$chunk->setError("Method required");

		$operand = $context->get("operand");
		switch ($method)
		{
			case "add":
				$result += $operand;
				break;
			case "subtract":
				$result -= $operand;
				break;
			case "multiply":
				$result *= $operand;
				break;
			case "divide":
				$result /= $operand;
				break;
			case "mod":
				$result %= $operand;
				break;
			case "abs":
				$result = abs($result);
				break;
			case "floor":
				$result = floor($result);
				break;
			case "ceil":
				$result = ceil($result);
				break;
			default:
				$chunk->setError("Unknown method: " . $method);
		}

		// No bodies means just write
		if($bodies == NULL || $bodies->block == NULL)
			return $chunk->write($result);
		else
		{
			// Just eval body with some special state
			return $chunk->render($bodies->block, $context->pushState(new Evaluate\State((object) ["__selectInfo" => (object) ["selectComparisonSatisfied" => false], "key" => $result])));
		}
	}
}
