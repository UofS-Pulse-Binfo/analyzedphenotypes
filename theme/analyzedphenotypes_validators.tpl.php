<?php
/**
 * @file
 * Master template file of analyzed phenotypes module validation result.
 *
 * Available variables:
 */

$ap_validators =  module_invoke_all('ap_validators');
$failed_counter = 0;
?>

<div id="ap-content-window-container">
  <div id="ap-content-window-title">
    Validation Result
  </div>

  <div id="ap-content-window-copy">
    <ul id="ap-content-window-list">

      <?php
        foreach($status as $validator => $result) {
          $type = $result['type'];
          $details = $result['details'];
          $details = rtrim($details, ', ');

          // Style each error based on validation result.
          $style = '';

          if ($type == 'passed') {
            $style = 'ap-passed';
            $title = 'Passed';
          }
          elseif ($type == 'failed') {
            $style = 'ap-failed';
            $title = 'Failed';

            $failed_counter++;
          }
          else {
            $style = 'ap-todo';
            $title = 'Pending';
          }

          print '<li class="' . $style . '" title="' . $title . '"><em>' . $ap_validators[$validator]['label'] . '</em></li>';

          // See if validator has more message about the error.
          if ($type == 'failed') {
            $error_message = $ap_validators[$validator]['error message'];

            if (!empty($error_message)) {

              // Check for additional info.
              if (!empty(trim($details))) {
                $error_message = str_replace('@replace', '<i class="ap-error-details">' . $details . '.</i>', $error_message);
              }

              print '<ul class="ap-content-window-sublist">';
              print '<li class="ap-sublist-item">' . $error_message . '</li>';
              print '</ul>';
            }
          }
        }
      ?>

    </ul>
  </div>
</div>

<?php
  // Keep the validation result and drupal set message error/success together.
  if ($failed_counter > 0) {
    $type = 'messages error';
    $message = 'The specified file could not be uploaded. See Validation Result for more information about the error.';
    $id = 'ap-validator-failed';
  }
  else {
    $type = 'messages status';
    $message = 'Your file uploaded successfully. Please click "Next Step" to continue.';
    $id = 'ap-validator-passed';
  }
?>

<div <?php print 'class="' . $type .'" id="' . $id . '"'; ?>>
  <?php print $message; ?>
</div>
