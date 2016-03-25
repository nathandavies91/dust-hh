<?hh // strict

namespace Dust\Evaluate;

use Dust\Ast\Ast;

class EvaluateException extends \Exception
{
	/**
	 * @var \Dust\Ast\Ast
	 */
	public Ast/Ast $ast;

	/**
	 * @param \Dust\Ast\Ast $ast
	 * @param string $message
	 */
	public function __construct(?Ast $ast = NULL, ?string $message = NULL): void {
		$this->ast = $ast;
		parent::__construct($message);
	}
}
