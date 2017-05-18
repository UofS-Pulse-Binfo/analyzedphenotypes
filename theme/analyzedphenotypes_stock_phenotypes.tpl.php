  <div class="ribbon ribbon-variant">
    <div class="banner">
      <div class="text">
        <?php print $data_crop; ?>
      </div>
    </div>
  </div>

  <div id="container-infographics">
    <?php
      foreach($data_categories as $k => $v) {
        $t = analyzedphenotypes_shorten_value($v);
        $title = $t . ' ' . ucfirst($k);
        $img = $path_to_img . strtolower($k) . '.gif';
    ?>
      <div class="container-data-category">
        <div class="container-round-icon">
          <div class="data-icon">
            <img src="<?php echo $img; ?>" height="45" widht="40" alt="<?php echo $k; ?>" title="<?php echo $k; ?>" />
          </div>
        </div>

        <div class="container-data-text">
          <em><?php echo $title; ?></em>
        </div>
      </div>
    <?php } ?>

    <div class="bg">&nbsp;</div>
  </div>


  <div id="container-title-bar" class="marker">
    <h2><?php print $stock_title; ?></h2>
  </div>


  <br />
  <!--
  2. TRAIT LIST.
  -->
  <div id="container-ap-contents">
    <div>
      <div class="window-title-icon">
        <div>&nbsp;</div>
      </div>

      <div class="container-window-title">
        <h2>Trait Distribution Chart</h2>
        <em>Select a trait to generate Trait Distribution Chart</em>
      </div>

      <div class="container-window">
        <div class="container-window-data-contents">
          <div id="ap-acs-form-elements-autocomplete-search" class="container-form-element-searchbox">
            &nbsp;
          </div>

          <div class="wrapper-ap-contents">
            <ul id="trait-list">
            <?php
              foreach($data_phenotypes as $t) {
                list($trait, $unit) = explode('(', $t);
                $x = mt_rand();

                echo '<li id="'.$x.'"><div class="icon-trait-leaf">&nbsp;</div> <em>' . $trait . '<br /><span>(' .$unit . '</span></em></li>';
              }
            ?>
            </ul>
          </div>
        </div>

        <div id="beanplot-container">&nbsp;</div>
      </div>
    </div>
  </div>
