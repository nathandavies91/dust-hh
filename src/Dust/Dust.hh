<?hh // strict

namespace Dust;

use Dust\Ast\Body;
use Dust\Evaluate\Evaluator;
use Dust\Parse\Parser;

class Dust implements \Serializable
{
    const string FILE_EXTENSION = ".dust";

    /**
     * @var \Dust\Parse\Parser
     */
    public Parser $parser = NULL;

    /**
     * @var \Dust\Evaluate\Evaluator
     */
    public Evaluator $evaluator = NULL;

    /**
     * @var array[string] => Ast\Body
     */
    public array<string> $templates = [];

    /**
     * @var array
     */
    public array
        $automaticFilters = [],
        $filters = [],
        $helpers = [],
        $includedDirectories = [];

    /**
     * @var object
     */
    public $autoloaderOverride;

    /**
     * @param Parser $parser
     * @param Evaluator $evaluator
     */
    public function __construct(?Parser $parser = NULL, ?Evaluator $evaluator = NULL): void {
        $this->parser = ($parser !== NULL) ? $parser : new Parser();
        $this->evaluator = ($evaluator !== NULL) ? $evaluator : new Evaluate\Evaluator($this);

        $this->filters = [
            "s"  => new Filter\SuppressEscape(),
            "h"  => new Filter\HtmlEscape(),
            "j"  => new Filter\JavaScriptEscape(),
            "u"  => new Filter\EncodeUri(),
            "uc" => new Filter\EncodeUriComponent(),
            "js" => new Filter\JsonEncode(),
            "jp" => new Filter\JsonDecode()
        ];
        
        $this->helpers = [
            "select"      => new Helper\Select(),
            "math"        => new Helper\Math(),
            "eq"          => new Helper\Eq(),
            "if"          => new Helper\IfHelper(),
            "lt"          => new Helper\Lt(),
            "lte"         => new Helper\Lte(),
            "gt"          => new Helper\Gt(),
            "gte"         => new Helper\Gte(),
            "default"     => new Helper\DefaultHelper(),
            "sep"         => new Helper\Sep(),
            "size"        => new Helper\Size(),
            "contextDump" => new Helper\ContextDump()
        ];

        $this->automaticFilters = [$this->filters["h"]];
    }

    /**
     * @param string $source
     * @param string $name
     * @return \Dust\Ast\Body|null
     */
    public function compile(string $source, string $name = NULL): ?Body {
        $parsed = $this->parser->parse($source);
        if ($name != NULL) $this->register($name, $parsed);

        return $parsed;
    }

    /**
     * @param string $path
     * @param string $basePath
     * @return \Dust\Ast\Body|null
     */
    public function compileFile(string $path, ?string $basePath = NULL): ?Body {
        // Resolve absolute path
        $absolutePath = $this->resolveAbsoluteDustFilePath($path, $basePath);

        if ($absolutePath == NULL)
            return NULL;
        
        // Just compile w/ the path as the name
        $compiled = $this->compile(file_get_contents($absolutePath), $absolutePath);
        $compiled->filePath = $absolutePath;

        return $compiled;
    }

    /**
     * @param      $source
     * @param null $name
     * @return callable
     */
    public function compileFn(string $source, ?string $name = NULL): callable {
        $parsed = $this->compile($source, $name);

        return function ($context) use ($parsed) {
            return $this->renderTemplate($parsed, $context);
        };
    }

    /**
     * @param string $name
     * @param string $basePath
     * @return Ast\Body|NULL
     */
    public function loadTemplate(string $name, ?string $basePath = NULL): ?Body {
        // If there is an override, use it instead
        if ($this->autoloaderOverride != NULL)
            return $this->autoloaderOverride->__invoke($name);
        
        // Is it there w/ the normal name?
        if (!isset($this->templates[$name]))
        {
            // What if I used the resolve file version of the name
            $name = $this->resolveAbsoluteDustFilePath($name, $basePath);
            // If name is null, then it's not around
            if ($name == NULL)
                return NULL;
            
            // If name is null and not in the templates array, put it there automatically
            if (!isset($this->templates[ $name ]))
                $this->compileFile($name, $basePath);
        }

        return $this->templates[$name];
    }

    /**
     * @param string         $name
     * @param \Dust\Ast\Body $template
     */
    public function register(string $name, Body $template) {
        $this->templates[$name] = $template;
    }

    /**
     * @param string $name
     * @param array  $context
     * @return string
     */
    public function render(string $name, ?array $context = []): string {
        return $this->renderTemplate($this->loadTemplate($name), $context);
    }

    /**
     * @param \Dust\Ast\Body $template
     * @param array          $context
     * @return string
     */
    public function renderTemplate(Body $template, ?array $context = []): string {
        return $this->evaluator->evaluate($template, $context);
    }

    /**
     * @param      $path
     * @param null $basePath
     * @return null|string
     */
    public function resolveAbsoluteDustFilePath(string $path, ?string $basePath = NULL): ?string {
        // Add extension if necessary
        if (substr_compare($path, self::FILE_EXTENSION, -5, 5) !== 0)
            $path .= self::FILE_EXTENSION;

        if ($basePath != NULL && ($possible = realpath($basePath . "/" . $path)) !== false)
            return $possible;

        // Try the current path
        $possible = realpath($path);
        if ($possible !== false)
            return $possible;

        // Now try each of the included directories
        for ($i = 0; $i < count($this->includedDirectories); $i++)
            if (($possible = realpath($this->includedDirectories[$i] . "/" . $path)) !== false) return $possible; 

        return NULL;
    }

    /**
     * @return string
     */
    public function serialize(): string {
        return serialize($this->templates);
    }

    /**
     * @param $data
     */
    public function unserialize($data): void {
        $this->templates = unserialize($data);
    }
}