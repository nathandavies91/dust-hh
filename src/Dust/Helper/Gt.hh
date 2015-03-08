<?hh // strict

namespace Dust\Helper;

class Gt extends Comparison
{
    /**
     * @param string $key
     * @param string $value
     * @return bool
     */
    public function isValid(string $key, string $value): bool {
        return $key > $value;
    }
}