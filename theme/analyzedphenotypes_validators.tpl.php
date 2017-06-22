<?php
/**
 * @file
 * Master template file of analyzed phenotypes module validation result.
 *
 * Available variables:
 */

$ap_validators =  module_invoke_all('ap_validators');
?>

<div id="ap-content-window-container">
  <div id="ap-content-window-title">
    Validation Result
  </div>

  <div id="ap-content-window-copy">
    <ul id="ap-content-window-list">

      <?php
        foreach($status as $validator => $result) {
          // Style each error based on validation result.
          $style = '';

          if ($result == 'passed') {
            $style = 'ap-passed';
          }
          elseif ($result == 'failed') {
            $style = 'ap-failed';
          }
          else {
            $style = 'ap-todo';
          }

          print '<li class="' . $style . '"><em>' . $ap_validators[$validator]['label'] . '</em></li>';

          // See if validator has more message about the error.
          if ($result == 'failed') {
            $error_message = $ap_validators[$validator]['error message'];

            if (!empty($error_message)) {
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
