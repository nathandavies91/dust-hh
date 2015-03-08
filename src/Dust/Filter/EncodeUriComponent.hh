<?hh // strict

namespace Dust\Filter;

class EncodeUriComponent implements Filter
{
    /**
     * @var array
     */
    public static array $replacers;

    /**
     * @param $item
     * @return object
     */
    public function apply($item): object {
        if (!is_string($item))
            return $item;

        return strtr(rawurlencode($item), EncodeUriComponent::$replacers);
    }
}

EncodeUriComponent::$replacers = [
    "%21" => "!",
    "%2A" => "*",
    "%27" => "'",
    "%28" => "(",
    "%29" => ")"
];