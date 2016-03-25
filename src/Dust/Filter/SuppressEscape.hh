<?hh // strict

namespace Dust\Filter;

class SuppressEscape implements Filter
{
	/**
	 * @param $item
	 * @return string
	 */
	public function apply($item): string {
		return $item;
	}
}
