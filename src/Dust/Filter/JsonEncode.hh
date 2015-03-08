<?hh // strict

namespace Dust\Filter;

class JsonEncode implements Filter
{
    /**
     * @param string $item
     * @return
     */
    public function apply($item): string {
        return json_encode($item);
    }
}