<?hh // strict

namespace Dust\Filter;

class JsonDecode implements Filter
{
    /**
     * @param string $item
     * @return string
     */
    public function apply($item): string {
        return json_decode($item);
    }
}