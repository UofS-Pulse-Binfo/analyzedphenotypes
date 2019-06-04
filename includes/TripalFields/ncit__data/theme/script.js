/**
 * @file
 * Manage NCIT Data field behavior
 */
(function ($) {
Drupal.behaviors.NCITData = {
  attach: function(context, settings) {
  // Auto select field or content of autocomplete field.
  $('#ap-search-autocomplete-textfield').focus();
  $('#ap-search-autocomplete-textfield').click(function() {
    $(this).select();
  });
}}}(jQuery));
