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


  <div id="container-ap-contents">
    <div>
      <div class="window-title-icon">
        <div class="info">&nbsp;</div>
      </div>

      <div class="container-window-title">
        <h2>Trait Definition/Description</h2>
        <em>Lorem ipsum dolor sit amet</em>
      </div>

      <div class="container-window">
        <div class="container-window-data-contents">
          <div class="ap-contents">
            <img class="ap-contents-left" src="<?php print $path_to_img . 'test-img.jpg' ?>" />

            <p>
              <h3>Days till 10% of Plants have 1/2 Pods Mature (R7; days)</h3>
              Before the pods dry out they lose their green pigmentation, often looking pale, but will still
              contain moisture, which you can feel when you touch the pod. Pods that are considered mature will
              have changed colour and be dry to the touch.
            </p>

            <p>
              Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi id ornare ex, sed malesuada magna.
              Donec laoreet dui at turpis imperdiet tristique. Nullam pretium hendrerit tortor non condimentum.
              Nullam sit amet placerat lorem, nec dictum eros. Suspendisse vehicula at arcu nec convallis.
              Pellentesque justo turpis, rhoncus et lorem.
            </p>

            <div class="ap-contents-clear"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
