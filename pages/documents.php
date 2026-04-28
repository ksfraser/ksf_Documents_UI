<?php
/**
 * Documents Management
 *
 * Document management page for FrontAccounting
 */

$page_security = 'SA_DOCUMENTS';
$path_to_root = "../../..";

include_once($path_to_root . "/includes/session.inc");
include_once($path_to_root . "/includes/ui.inc");
include_once($path_to_root . "/modules/FA_Documents/includes/documents_db.inc");
include_once($path_to_root . "/modules/FA_Documents/includes/documents_ui.inc");

$js = "";
if ($SysPrefs->use_popup_windows)
    $js .= get_js_open_window(900, 600);

page(_($help_context = "Documents"), false, false, "", $js);

//--------------------------------------------------------------------------------------------

start_table(TABLESTYLE_NOBORDER);
start_row();
documents_navbar();
end_row();
end_table();

echo '<br>';

// Get section from URL
$section = isset($_GET['section']) ? $_GET['section'] : 'list';

switch ($section) {
    case 'view':
        $doc_id = isset($_GET['doc_id']) ? $_GET['doc_id'] : 0;
        display_document_view($doc_id);
        break;
    case 'edit':
        $doc_id = isset($_GET['doc_id']) ? $_GET['doc_id'] : 0;
        display_document_edit($doc_id);
        break;
    case 'attachments':
        $doc_id = isset($_GET['doc_id']) ? $_GET['doc_id'] : 0;
        display_document_attachments($doc_id);
        break;
    case 'list':
    default:
        display_documents_list();
        break;
}

end_page(true);