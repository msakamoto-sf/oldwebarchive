<?php
require_once("HTML/QuickForm.php");
require_once("HTML/QuickForm/Renderer/ArraySmarty.php");

define('QF_SMARTY_RENDERER_LIGHT_REQUIRED', 0x1);
define('QF_SMARTY_RENDERER_LIGHT_ERROR', 0x1 << 1);
define('QF_SMARTY_RENDERER_LIGHT_REQUIRED_ERROR', 
	QF_SMARTY_RENDERER_LIGHT_REQUIRED | QF_SMARTY_RENDERER_LIGHT_ERROR);

/**
 * QuickForm Smarty Renderer "Light"
 * <p>
 * Smartyレンダラでrequired/error時のlabel/html要素の独自レンダリング実装</p>
 * <p>
 * 各要素のレンダリング時に、「エラー発生」「require要素」の条件組み合わせに
 * 応じて、テンプレートを切り替えます。</p>
 * <p>
 * レンダリングできるのはhtml要素とlabel要素です。テンプレート文字列には次の
 * 文字を埋め込むことにより、デフォルトの各要素で置換されます。
 * <ul>
 * <li>{html} - デフォルトのフォーム要素HTML</li>
 * <li>{label} - デフォルトのフォームLabel</li>
 * <li>{error} - addRuleで指定したエラー文字列</li>
 * </ul></p>
 * <p>
 * テンプレートはコンストラクタで指定できます。指定が無い場合、デフォルト
 * として次の配列が使用されます。
 * <code>
 * array(
 *     QF_SMARTY_RENDERER_LIGHT_REQUIRED => array(
 *            'label' => '{label}<font color="red">*</font>',
 *        ),
 *        QF_SMARTY_RENDERER_LIGHT_ERROR => array(
 *            'label' => '<font color="red">{label}</font>',
 *            'html' => '{html}<br />{error}',
 *        ),
 *        QF_SMARTY_RENDERER_LIGHT_REQUIRED_ERROR => array(
 *            'label' => '<font color="red">{label}</font>',
 *            'html' => '{html}<br />{error}',
 *        ),
 *    );
 * </code></p>
 * <p>
 * エラーもrequiredも発生しないときはこのテンプレートは使用されません。<br />
 * エラーのみならQF_SMARTY_RENDERER_LIGHT_ERRORが使用されます。<br />
 * requireのみならQF_SMARTY_RENDERER_LIGHT_REQUIREDが使用されます。<br />
 * エラーが発生しておりなおかつrequiredな要素なら
 * QF_SMARTY_RENDERER_LIGHT_REQUIRED_ERRORが使用されます。<br /></p>
 * <p>
 * テンプレートはnewしたあと、setCostomizedTemplate()でセットすることも
 * 可能です。</p>
 * 
 * @author Akira Kaze <feng-jing-gsyc-2s@glamenv-septzen.net>
 */
class QFSmartyRendererLight extends HTML_QuickForm_Renderer_ArraySmarty
{

	/**
	 * カスタマイズテンプレート
	 *
	 * @type array
	 * @access private
	 */
	var $_customized_template = array(
		QF_SMARTY_RENDERER_LIGHT_REQUIRED => array(
			'label' => '{label}<font color="red">*</font>',
		),
		QF_SMARTY_RENDERER_LIGHT_ERROR => array(
			'label' => '<font color="red">{label}</font>',
			'html' => '{html}<br />{error}',
		),
		QF_SMARTY_RENDERER_LIGHT_REQUIRED_ERROR => array(
			'label' => '<font color="red">{label}</font>',
			'html' => '{html}<font color="red">*</font><br />{error}',
		),
	);

   /**
    * Constructor
    *
    * @param  object  reference to the Smarty template engine instance
    * @param  bool    true: render an array of labels to many labels, $key 0 to 'label' and the oterh to "label_$key"
    * @param array カスタマイズテンプレート
    * @access public
    */
	function QFSmartyRendererLight
		(&$tpl, $staticLabels = false, $templates = null)
	{
		$this->HTML_QuickForm_Renderer_ArraySmarty($tpl, $staticLabels);
		if(!is_null($templates)) {
			$this->_customized_template = $templates;
		}
	}

	/**
	 * カスタマイズテンプレートを設定する。
	 *
	 * @param array カスタマイズテンプレート
	 * @access public
	 */
	function setCustomizedTemplate($templates)
	{
		$this->_customized_template = $templates;
	}

	/**
	 * required or エラー発生した要素のみ呼ばれる。
	 *
	 * @param integer モード
	 * @param string ラベル文字列への参照
	 * @param string フォーム出力HTML文字列への参照
	 * @param string エラー発生時の文字列
	 * @access private
	 */
	function _renderCustomized($mode, &$label, &$html, $error)
	{
		if(!isset($this->_customized_template[$mode])) return;
		$templates = $this->_customized_template[$mode];

		$_label = $label;
		$_html = $html;

		$searches = array('{label}', '{html}');
		$replaces = array($_label, $_html);
		if($mode & QF_SMARTY_RENDERER_LIGHT_ERROR) {
			$searches[] = '{error}';
			$replaces[] = $error;
		}
		if(isset($templates['label'])) {
			$label = str_replace($searches, $replaces, $templates['label']);
		}
		if(isset($templates['html'])) {
			$html = str_replace($searches, $replaces, $templates['html']);
		}
	}


   /**
    * Creates an array representing an element containing
    * the key for storing this
    *
    * @access private
    * @param  object    An HTML_QuickForm_element object
    * @param  bool      Whether an element is required
    * @param  string    Error associated with the element
    * @return array
    */
    function _elementToArray(&$element, $required, $error)
    {
        $ret = parent::_elementToArray($element, $required, $error);

        if ('group' == $ret['type']) {
            $ret['html'] = $element->toHtml();
            // we don't need the elements, see the array structure
            unset($ret['elements']);
        }

		$mode = 0;
		if(!empty($required)) $mode |= QF_SMARTY_RENDERER_LIGHT_REQUIRED;
		if(!empty($error)) $mode |= QF_SMARTY_RENDERER_LIGHT_ERROR;
		if($mode > 0) {
			$this->_renderCustomized($mode, 
				$ret['label'], $ret['html'], $error);
		}
        // create keys for elements grouped by native group or name
        if (strstr($ret['name'], '[') or $this->_currentGroup) {
            preg_match('/([^]]*)\\[([^]]*)\\]/', $ret['name'], $matches);
            if (isset($matches[1])) {
                $sKeysSub = substr_replace($ret['name'], '', 0, strlen($matches[1]));
                $sKeysSub = str_replace(
                    array('['  ,   ']', '[\'\']'),
                    array('[\'', '\']', '[]'    ),
                    $sKeysSub
                );
                $sKeys = '[\'' . $matches[1]  . '\']' . $sKeysSub;
            } else {
                $sKeys = '[\'' . $ret['name'] . '\']';
            }
            // special handling for elements in native groups
            if ($this->_currentGroup) {
                // skip unnamed group items unless radios: no name -> no static access
                // identification: have the same key string as the parent group
                if ($this->_currentGroup['keys'] == $sKeys and 'radio' != $ret['type']) {
                    return false;
                }
                // reduce string of keys by remove leading group keys
                if (0 === strpos($sKeys, $this->_currentGroup['keys'])) {
                    $sKeys = substr_replace($sKeys, '', 0, strlen($this->_currentGroup['keys']));
                }
            }
        // element without a name
        } elseif ($ret['name'] == '') {
            $sKeys = '[\'element_' . $this->_elementIdx . '\']';
        // other elements
        } else {
            $sKeys = '[\'' . $ret['name'] . '\']';
        }
        // for radios: add extra key from value
        if ('radio' == $ret['type'] and substr($sKeys, -2) != '[]') {
            $sKeys .= '[\'' . $ret['value'] . '\']';
        }
        $this->_elementIdx++;
        $ret['keys'] = $sKeys;
        return $ret;
    } // end func _elementToArray

}
?>
