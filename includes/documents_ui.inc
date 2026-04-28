<?php
/**
 * Documents UI Functions
 *
 * UI helper functions for Documents module in FrontAccounting
 */

function documents_navbar(): void
{
    global $allowed_paths;
    
    $current_section = isset($_GET['section']) ? $_GET['section'] : 'list';
    
    start_table(TABLESTYLE);
    start_row();
    
    echo "<td><a href='?section=list' class='nav' " . ($current_section == 'list' ? "style='font-weight:bold;'" : "") . ">" . _("All Documents") . "</a></td>";
    echo "<td><a href='?section=list&type=Policy' class='nav'>" . _("Policies") . "</a></td>";
    echo "<td><a href='?section=list&type=Contract' class='nav'>" . _("Contracts") . "</a></td>";
    echo "<td><a href='?section=list&type=Form' class='nav'>" . _("Forms") . "</a></td>";
    echo "<td><a href='?section=expiring' class='nav'>" . _("Expiring Soon") . "</a></td>";
    echo "<td><a href='?section=pending' class='nav'>" . _("Pending Ack") . "</a></td>";
    
    end_row();
    end_table();
}

function display_documents_list(): void
{
    $type = isset($_GET['type']) ? $_GET['type'] : null;
    $search = isset($_GET['search']) ? $_GET['search'] : '';
    
    start_table(TABLESTYLE);
    
    if (user_has_permission('SA_DOCUMENTCREATE')) {
        echo "<tr><td colspan='6' align='right'><a href='?section=edit&doc_id=0'>" . _("Add New Document") . "</a></td></tr>";
    }
    
    table_header([
        _('ID'),
        _('Title'),
        _('Type'),
        _('Status'),
        _('Created'),
        _('Expires'),
    ]);
    
    $docs = get_documents_list($type, $search);
    
    while ($doc = db_fetch($docs)) {
        alt_table_row($doc);
        
        label_cell($doc['id']);
        
        $link = "?section=view&doc_id=" . $doc['id'];
        label_cell("<a href='$link'>" . $doc['title'] . "</a>");
        
        label_cell($doc['type']);
        
        label_cell($doc['status']);
        
        label_cell(sql2date($doc['created_at']));
        
        label_cell($doc['expires_at'] ? sql2date($doc['expires_at']) : '-');
    }
    
    end_table(1);
}

function display_document_view(int $doc_id): void
{
    $doc = get_document($doc_id);
    
    if (!$doc) {
        display_error(_("Document not found"));
        return;
    }
    
    start_table(TABLESTYLE);
    
    table_row(_("Title:"), $doc['title']);
    table_row(_("Type:"), $doc['type']);
    table_row(_("Status:"), $doc['status']);
    table_row(_("Created:"), sql2date($doc['created_at']));
    table_row(_("Requires Acknowledgment:"), $doc['ack_required'] == 'yes' ? _("Yes") : _("No"));
    if ($doc['ack_required'] == 'yes') {
        table_row(_("Acknowledgment Deadline:"), $doc['ack_deadline'] ? sql2date($doc['ack_deadline']) : '-');
    }
    table_row(_("Expires:"), $doc['expires_at'] ? sql2date($doc['expires_at']) : '-');
    
    if ($doc['entity_type']) {
        table_row(_("Linked To:"), $doc['entity_type'] . " #" . $doc['entity_id']);
    }
    
    end_table();
    
    h2(_("Attachments"));
    
    start_table(TABLESTYLE);
    
    table_header([
        _('File'),
        _('Uploaded'),
        _('Action'),
    ]);
    
    $attachments = get_document_attachments($doc_id);
    
    while ($att = db_fetch($attachments)) {
        alt_table_row($att);
        
        $download_link = $path_to_root . "/modules/FA_Documents/downloads.php?id=" . $att['id'];
        label_cell("<a href='$download_link' target='_blank'>" . $att['file_name'] . "</a>");
        
        label_cell(sql2date($att['uploaded_at']));
        
        if (user_has_permission('SA_DOCUMENTDELETE')) {
            $delete_link = $path_to_root . "/modules/FA_Documents/includes/delete_attachment.php?id=" . $att['id'];
            label_cell("<a href='$delete_link' onclick='return confirm(\"Delete this attachment?\");'>" . _("Delete") . "</a>");
        }
    }
    
    end_table(1);
    
    if (user_has_permission('SA_DOCUMENTUPLOAD')) {
        echo "<br><a href='?section=attachments&doc_id=" . $doc_id . "'>" . _("Manage Attachments") . "</a>";
    }
}

function display_document_edit(int $doc_id): void
{
    global $Ajax;
    
    $doc = $doc_id > 0 ? get_document($doc_id) : null;
    
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
        $title = $_POST['title'] ?? '';
        $type = $_POST['type'] ?? 'Other';
        $expires_at = $_POST['expires_at'] ?? null;
        $ack_required = $_POST['ack_required'] ?? 'no';
        $ack_deadline = $_POST['ack_deadline'] ?? null;
        $entity_type = $_POST['entity_type'] ?? '';
        $entity_id = $_POST['entity_id'] ?? null;
        
        if ($doc_id > 0) {
            update_document($doc_id, $title, $type, $expires_at, $ack_required, $ack_deadline, $entity_type, $entity_id);
            display_notification(_("Document updated"));
        } else {
            $new_id = create_document($title, $type, $expires_at, $ack_required, $ack_deadline, $entity_type, $entity_id);
            display_notification(_("Document created"));
            meta_refresh(null, "?section=edit&doc_id=" . $new_id);
        }
        
        $Ajax->activate('content_area');
    }
    
    start_form(true);
    
    start_table(TABLESTYLE);
    
    text_row(_("Title:"), 'title', $doc['title'] ?? '', 50);
    
    $type_options = [
        'Policy' => _('Policy'),
        'Contract' => _('Contract'),
        'Form' => _('Form'),
        'Training' => _('Training'),
        'Handbook' => _('Handbook'),
        'Other' => _('Other'),
    ];
    select_row(_("Type:"), 'type', $type_options, $doc['type'] ?? 'Other');
    
    date_row(_("Expires:"), 'expires_at', $doc['expires_at'] ?? '');
    
    yes_no_list_row(_("Requires Acknowledgment:"), 'ack_required', $doc['ack_required'] ?? 'no');
    
    date_row(_("Acknowledgment Deadline:"), 'ack_deadline', $doc['ack_deadline'] ?? '');
    
    $entity_type_options = [
        '' => _('-- Select --'),
        'employee' => _('Employee'),
        'project' => _('Project'),
        'customer' => _('Customer'),
    ];
    select_row(_("Entity Type:"), 'entity_type', $entity_type_options, $doc['entity_type'] ?? '');
    
    smallint_row(_("Entity ID:"), 'entity_id', $doc['entity_id'] ?? null);
    
    end_table(1);
    
    submit_center('submit', $doc_id > 0 ? _("Update Document") : _("Create Document"));
    
    end_form();
}

function display_document_attachments(int $doc_id): void
{
    if ($_SERVER['REQUEST_METHOD'] == 'POST' && !empty($_FILES['attachment'])) {
        $file = $_FILES['attachment'];
        
        if ($file['error'] === UPLOAD_ERR_OK) {
            $upload_dir = $path_to_root . "/modules/FA_Documents/attachments/";
            
            if (!is_dir($upload_dir)) {
                mkdir($upload_dir, 0755, true);
            }
            
            $filename = basename($file['name']);
            $target = $upload_dir . $filename;
            
            if (move_uploaded_file($file['tmp_name'], $target)) {
                add_document_attachment($doc_id, $target, $filename);
                display_notification(_("File uploaded"));
            } else {
                display_error(_("Failed to upload file"));
            }
        }
    }
    
    $doc = get_document($doc_id);
    
    start_form(true, 'POST', '', 'attachment-form', 'enctype="multipart/form-data"');
    
    hidden('section', 'attachments');
    hidden('doc_id', $doc_id);
    
    file_row(_("Upload File:"), 'attachment', '');
    
    end_table(1);
    
    submit_center('upload', _("Upload"));
    
    end_form();
}