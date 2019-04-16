<?php
/**
 * @file
 * Contains data exporter interface of this module.
 */



// AJAX fields API in Data donwloader module.
module_load_include('inc', 'analyzedphenotypes', 'api/analyzedphenotypes.formfields.api');


/**
 * FUNCTION CALLBACK
 * Construct data downloader form
 */
function analyzedphenotypes_downloaddata_form($form, &$form_state) {
  // Include CSS and JavaScript.
  $path_theme = drupal_get_path('module', 'analyzedphenotypes') . '/theme/';
  $form['#attached']['css'][] = $path_theme . 'css/analyzedphenotypes.downloader.global.css';
  $form['#attached']['js'] [] = $path_theme . 'js/analyzedphenotypes.downloader.script.js';

  drupal_add_library('system', 'ui.autocomplete');

  // Field names.
  // Same name will be used for hooks, js etc.
  $fldname = ap_construct_download_fieldnames();

  // F. 1.
  $fld_experiment      = $fldname['experiment']['base'];
  $fld_genus           = $fldname['genus']['base'];
  $fld_species         = $fldname['species']['base'];
  $fld_traits          = $fldname['traits']['base'];
  // F. 2.
  $fld_year            = $fldname['year']['base'];
  $fld_location        = $fldname['location']['base'];
  $fld_germplasmtype   = $fldname['germplasmtype']['base'];
  $fld_germplasm       = $fldname['germplasm']['base'];
  $fld_missingdata     = $fldname['missingdata']['base'];
  // F. 3.
  $fld_filetype        = $fldname['filetype']['base'];
  $fld_averagerep      = $fldname['averagerep']['base'];
  $fld_rfriendly       = $fldname['rfriendly']['base'];
  $fld_columnheaders   = $fldname['columnheaders']['base'];

  // Field form properties.
  $field_prop = array(
    //
    $fld_experiment    => array('options'),
    $fld_genus         => array('disabled' => TRUE, 'options' => array(), 'empty'   => t('- Select -')),
    $fld_species       => array('disabled' => TRUE, 'options' => array(), 'empty'   => t('- Select -')),
    $fld_traits        => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    //
    $fld_year          => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_location      => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_germplasmtype => array('disabled' => TRUE, 'options' => array(), 'default' => array(0)),
    $fld_germplasm     => array('disabled' => TRUE, 'options' => array(), 'default' => ''),
    $fld_missingdata   => array('disabled' => TRUE, 'options' => array(), 'default' => '100%'),
    //
    $fld_filetype      => array('disabled' => TRUE),
    $fld_averagerep    => array('disabled' => TRUE),
    $fld_rfriendly     => array('disabled' => TRUE),
  );

  // Main AJAX Wrapper.
  $AJAX_wrapper_id = 'ap-AJAX-wrapper-main';
  // Use this character to separate entries in a string.
  $delimiter = '~';

  // Global AJAX settings.
  // This array element will add AJAX to any field.
  $AJAX_callback['#ajax'] = array(
    'event' => 'change',
    'callback' => 'ap_send_ajaxrequest',
    'case' => 'main',
    'wrapper' => $AJAX_wrapper_id,
    'progress' => array('type' => 'none', 'message' => ''),
  );

  // Main AJAX wrapper. Any field wrapped in this container
  // will be processed by AJAX request.
  $AJAX_wrapper = 'ap_AJAX_wrapper_main';
  $form[$AJAX_wrapper] = array(
    '#prefix' => '<div id="' . $AJAX_wrapper_id . '">',
    '#suffix' => '</div>'
  );

  // HANDLE AJAX:
  // Initialize form field value and state.


  // Create list of default and optional column headers.
  // Will return Experiment, Trait Name, Germplasm Name, Year, Location, Replicate, Data Collector, Country of Origin.
  $headers = ap_create_columnheaders('columns_downloadable');
  $default_headers = array_combine($headers, $headers);
  $optional_headers = array();

  // Move Data Collector and Origin to optional column headers.
  $optional_headers['Data Collector'] = 'Data Collector';
  unset($default_headers['Data Collector']);

  $optional_headers['Origin'] = 'Country of Origin';
  unset($default_headers['Country of Origin']);

  // By default form averages rep - replicate is not required.
  unset($default_headers['Replicate']);

  // Contain all field values.
  $field_values = null;
  // Contain all cache values of fields.
  $cache_values = null;

  // Submit button. This ensure the submit button gets enabled
  // when this variable is set to 1.
  $btn_ok = 0;

  // Manage trait information variables.
  $t_options = array();
  $trait_method_unit_id = array();


  // Handle AJAX. form state values has values.
  if (isset($form_state['values'])) {
    //
    if ($form_state['values'][ $fldname[$fld_experiment]['field'] ] != '') {
      /// Experiment selected.
      $e = $form_state['values'][ $fldname[$fld_experiment]['field'] ];
      $field_values[$fld_experiment] = analyzedphenotypes_checkboxesvalue($e);

      // Enable avaerage rep and r friendly options.
      if (count($field_values[$fld_experiment])) {
        $field_prop[$fld_filetype]  ['disabled'] = FALSE;
        $field_prop[$fld_averagerep]['disabled'] = FALSE;
        $field_prop[$fld_rfriendly] ['disabled'] = FALSE;
      }

      // Project selected. Test if multiple projects selected.
      if (count($field_values[$fld_experiment]) > 1) {
        // MANY EXPERIMENTS.

        // Enable submit button.
        $btn_ok = 1;

        // #Genus.
        $field_values[$fld_genus] = 'all';

        $field_prop[$fld_genus]['options'] = array($field_values[$fld_genus] => 'All Genus');
        $field_prop[$fld_genus]['disabled'] = FALSE;
        $field_prop[$fld_genus]['empty'] = null;


        // Convert all value for genus and species into values.
        $all_genus = ap_populate_fieldgenus(array(
          'project_id' => $field_values[$fld_experiment]
        ));

        $all_species = ap_populate_fieldspecies(array(
          'project_id' => $field_values[$fld_experiment],
          'genus' => $all_genus
        ));

        // #Species.
        $field_prop[$fld_species]['default'] = array_keys($all_species);
        $field_prop[$fld_species]['options'] = $all_species;
        $field_prop[$fld_species]['disabled'] = FALSE;

        // #Traits.
        $options = ap_populate_fieldtraits(array(
          'project_id' => $field_values[$fld_experiment],
        ));

        if ($options) {
          foreach($options as $i => $w) {
            // Trait Name (Short Method Name; Unit)
            $t_options[ ($i + 1) ] = $w->name . ' (' . $w->method . '; ' . $w->unit . ')';
            // Trait Name Id, Short Method Name Id, Unit Id.
            $trait_method_unit_id[ ($i + 1) ] = $w->name_id . ',' . $w->method_id . ',' . $w->unit_id;
          }

          $field_values[$fld_traits] = array_keys($t_options);

          $field_prop[$fld_traits]['default'] = $field_values[$fld_traits];
          $field_prop[$fld_traits]['options'] = $t_options;
          $field_prop[$fld_traits]['disabled'] = FALSE;

          // #Year.
          unset($options);
          $options = ap_populate_fieldyear(array(
            'project_id' => $field_values[$fld_experiment],
            'traits' => $field_values[$fld_traits],
            'trait_method_unit_id' => $trait_method_unit_id,
            'genus' => $all_genus,
            'species' => $all_species
          ));

          if ($options) {
            $field_values[$fld_year] = array_keys($options);

            $field_prop[$fld_year]['default'] = $field_values[$fld_year];
            $field_prop[$fld_year]['options'] = $options;
            $field_prop[$fld_year]['disabled'] = FALSE;

            // #Location.
            unset($options);
            $options = ap_populate_fieldlocation(array(
              'project_id' => $field_values[$fld_experiment],
              'traits' => $field_prop[$fld_traits]['default'],
              'trait_method_unit_id' => $trait_method_unit_id,
              'year' => $field_prop[$fld_year]['default']
            ));

            if ($options) {
              $field_values[$fld_location] = array_keys($options);

              $field_prop[$fld_location]['default'] = $field_values[$fld_location];
              $field_prop[$fld_location]['options'] = $options;
              $field_prop[$fld_location]['disabled'] = FALSE;

              // #Germplasm Type.
              unset($options);
              $options = ap_populate_fieldgermplasmtype(array(
                'project_id' => $field_values[$fld_experiment],
                'traits' => $field_prop[$fld_traits]['default'],
                'trait_method_unit_id' => $trait_method_unit_id,
                'year' => $field_prop[$fld_year]['default'],
                'location' => $field_prop[$fld_location]['default']
              ));

              if ($options) {
                $field_values[$fld_germplasmtype] = array_keys($options);

                $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                $field_prop[$fld_germplasmtype]['options'] = $options;
                $field_prop[$fld_germplasmtype]['disabled'] = FALSE;

                // Germplasm.
                unset($options);
                $options = ap_populate_fieldgermplasmlist(array(
                  'project_id' => $field_values[$fld_experiment]
                ));

                if ($options) {
                  $field_prop[$fld_germplasm]['default'] = '';
                  $field_prop[$fld_germplasm]['options'] = $options;
                  $field_prop[$fld_germplasm]['disabled'] = TRUE;

                  $field_prop[$fld_missingdata]['default'] = '';
                  $field_prop[$fld_missingdata]['disabled'] = TRUE;
                // end Germplasm.
                }
              // end Germplasm Type.
              }
            // end Location.
            }
          // end Year.
          }
        // end Traits.
        }

      // END MANY EXPERIMENTS.
      }
      else {
        // ONE EXPERIMENT.

        $field_values[$fld_experiment] = current($field_values[$fld_experiment]);

        // Experiment is optional with single project and not average rep.
        if ($field_values[$fld_experiment] > 0 && $form_state['values'][ $fldname[$fld_averagerep]['field'] ] == 0) {
          unset($default_headers['Experiment']);
          $optional_headers['Experiment'] = 'Experiment';
        }

        // #Genus.
        $options = ap_populate_fieldgenus(array(
          'project_id' => $field_values[$fld_experiment]
        ));

        if ($options) {
          $field_prop[$fld_genus]['options'] = $options;
          $field_prop[$fld_genus]['disabled'] = FALSE;
          $field_prop[$fld_genus]['empty'] = null;

          $field_values[$fld_genus] = current($options);

          // #Species
          unset($options);

          $options = ap_populate_fieldspecies(array(
            'project_id' => $field_values[$fld_experiment],
            'genus' => $field_values[$fld_genus]
          ));

          if ($options) {
            $field_prop[$fld_species]['disabled'] = FALSE;
            $field_prop[$fld_species]['options'] = $options;
          }

          unset($options);
        }

        // Cache value selected. This will allow us to compare cache values
        // with current values returned from selecting different filters in the form.
        // NOTE: _cachevalue() function returns string as a result of imploding the array.
        $form['values'][ $fldname[$fld_experiment]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_experiment]);
        $form['values'][ $fldname[$fld_genus]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_genus]);

        $cache_values[$fld_experiment] = $form_state['values'][ $fldname[$fld_experiment]['cache'] ];
        $cache_values[$fld_genus] = $form_state['values'][ $fldname[$fld_genus]['cache'] ];

        // AJAX - same experiment and genus.
        if ($cache_values[$fld_experiment] == $field_values[$fld_experiment] &&
            $cache_values[$fld_genus] == $field_values[$fld_genus]) {

          /// Species selected - Create traits options.
          if ($form_state['values'][ $fldname[$fld_species]['field'] ] != '') {
            $s = $form_state['values'][ $fldname[$fld_species]['field'] ];
            $field_values[$fld_species] = analyzedphenotypes_checkboxesvalue($s);

            $options = ap_populate_fieldtraits(array(
              'project_id' => $field_values[$fld_experiment],
              'genus' => $field_values[$fld_genus],
              'species' => $field_values[$fld_species]
            ));

            // #Traits.
            if ($options) {
              foreach($options as $i => $w) {
                // Trait Name (Short Method Name; Unit)
                $t_options[ ($i + 1) ] = $w->name . ' (' . $w->method . '; ' . $w->unit . ')';
                // Trait Name Id, Short Method Name Id, Unit Id.
                $trait_method_unit_id[ ($i + 1) ] = $w->name_id . ',' . $w->method_id . ',' . $w->unit_id;
              }

              $field_prop[$fld_traits]['options'] = $t_options;
              $field_prop[$fld_traits]['disabled'] = FALSE;
              $field_values[$fld_traits] = array_keys($t_options);

              if (count($field_values[$fld_species]) > 1) {
                // Many species selected.
                $btn_ok = 1;

                $field_prop[$fld_traits]['default'] = $field_values[$fld_traits];

                // Year, Location, Germplasm type.
                $options = ap_populate_fieldyear(array(
                  'project_id' => $field_values[$fld_experiment],
                  'traits' => $field_values[$fld_traits],
                  'trait_method_unit_id' => $trait_method_unit_id,
                  'genus' => $field_values[$fld_genus],
                  'species' => $field_values[$fld_species]
                ));

                if ($options) {
                  $field_prop[$fld_year]['options'] = $options;
                  $field_prop[$fld_year]['disabled'] = FALSE;
                  $field_prop[$fld_year]['default'] = array_keys($options);

                  $field_values[$fld_year] = array_keys($options);

                  $options = ap_populate_fieldlocation(array(
                    'project_id' => $field_values[$fld_experiment],
                    'genus' => $field_values[$fld_genus],
                    'species' => $field_values[$fld_species],
                    'traits' => $field_values[$fld_traits],
                    'trait_method_unit_id' => $trait_method_unit_id,
                    'year' => $field_values[$fld_year],
                  ));

                  if ($options) {
                    $field_prop[$fld_location]['options'] = $options;
                    $field_prop[$fld_location]['disabled'] = FALSE;

                    $field_values[$fld_location] = array_keys($options);
                    $field_prop[$fld_location]['default'] = $field_values[$fld_location];

                    $options = ap_populate_fieldgermplasmtype(array(
                      'project_id' => $field_values[$fld_experiment],
                      'genus' => $field_values[$fld_genus],
                      'species' => $field_values[$fld_species],
                      'traits' => $field_values[$fld_traits],
                      'trait_method_unit_id' => $trait_method_unit_id,
                      'year' => $field_values[$fld_year],
                      'location' => $field_values[$fld_location]
                    ));

                    if ($options) {
                      $field_values[$fld_germplasmtype] = array_keys($options);

                      $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                      $field_prop[$fld_germplasmtype]['options'] = $options;
                      $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                    }
                  }
                }
              }
            }

            unset($options);

            // Cache value selected.
            $form['values'][ $fldname[$fld_species]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_species]);
            $cache_values[$fld_species] = $form_state['values'][ $fldname[$fld_species]['cache'] ];


            // AJAX - same experiment, genus and species.
            if ($cache_values[$fld_species] == implode('_', $field_values[$fld_species])) {

              ///// Traits selected - Create year options.
              if ($form_state['values'][ $fldname[$fld_traits]['field'] ] != '') {
                $t = $form_state['values'][ $fldname[$fld_traits]['field'] ];
                $field_values[$fld_traits] = analyzedphenotypes_checkboxesvalue($t);

                if (count($field_values[$fld_traits]) > 0) {
                  // User selected some traits, enable submit button.
                  $btn_ok = 1;

                  $options = ap_populate_fieldyear(array(
                    'project_id' => $field_values[$fld_experiment],
                    'traits' => $field_values[$fld_traits],
                    'trait_method_unit_id' => $trait_method_unit_id,
                    'genus' => $field_values[$fld_genus],
                    'species' => $field_values[$fld_species]
                  ));

                  if ($options) {
                    $field_prop[$fld_year]['options'] = $options;
                    $field_prop[$fld_year]['disabled'] = FALSE;

                    // Year, Location and Germplasm Type.
                    if (count($field_values[$fld_traits]) > 1) {
                      // Many traits selected.
                      $field_values[$fld_year] = array_keys($options);

                      $field_prop[$fld_year]['default'] = $field_values[$fld_year];
                      $options = ap_populate_fieldlocation(array(
                        'project_id' => $field_values[$fld_experiment],
                        'genus' => $field_values[$fld_genus],
                        'species' => $field_values[$fld_species],
                        'traits' => $field_values[$fld_traits],
                        'trait_method_unit_id' => $trait_method_unit_id,
                        'year' => $field_values[$fld_year]
                      ));

                      if ($options) {
                        $field_values[$fld_location] = array_keys($options);

                        $field_prop[$fld_location]['default'] = $field_values[$fld_location];
                        $field_prop[$fld_location]['options'] = $options;
                        $field_prop[$fld_location]['disabled'] = FALSE;

                        $options = ap_populate_fieldgermplasmtype(array(
                          'project_id' => $field_values[$fld_experiment],
                          'genus' => $field_values[$fld_genus],
                          'species' => $field_values[$fld_species],
                          'traits' => $field_values[$fld_traits],
                          'trait_method_unit_id' => $trait_method_unit_id,
                          'year' => $field_values[$fld_year],
                          'location' => $field_values[$fld_location]
                        ));

                        if ($options) {
                          $field_values[$fld_germplasmtype] = array_keys($options);

                          $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                          $field_prop[$fld_germplasmtype]['options'] = $options;
                          $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                        }
                      }
                    }
                  }

                  unset($options);

                  // Cache value selected.
                  $form['values'][ $fldname[$fld_traits]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_traits]);
                  $cache_values[$fld_traits] = $form_state['values'][ $fldname[$fld_traits]['cache'] ];

                  // AJAX - same experiment, genus, species and traits.
                  if ($cache_values[$fld_traits] == implode('_', $field_values[$fld_traits])) {

                    ////// Year selected - Create location options.
                    if ($form_state['values'][ $fldname[$fld_year]['field'] ] != '') {
                      $y = $form_state['values'][ $fldname[$fld_year]['field'] ];

                      // Many traits, use all year. Otherwise, use what user checked.
                      if (count($field_values[$fld_traits]) > 1) {
                        $field_values[$fld_year] = $field_values[$fld_year];
                      }
                      else {
                        $field_values[$fld_year] = analyzedphenotypes_checkboxesvalue($y);
                      }

                      if (count($field_values[$fld_year]) > 0) {
                        $options = ap_populate_fieldlocation(array(
                          'project_id' => $field_values[$fld_experiment],
                          'genus' => $field_values[$fld_genus],
                          'species' => $field_values[$fld_species],
                          'traits' => $field_values[$fld_traits],
                          'trait_method_unit_id' => $trait_method_unit_id,
                          'year' => $field_values[$fld_year]
                        ));

                        if ($options) {
                          $field_prop[$fld_location]['options'] = $options;
                          $field_prop[$fld_location]['disabled'] = FALSE;

                          // Location and Germplasm Type.
                          if (count($field_values[$fld_year]) > 1) {
                            // Many years selected.
                            $field_values[$fld_location] = array_keys($options);

                            $field_prop[$fld_location]['default'] = $field_values[$fld_location];
                            $options = ap_populate_fieldgermplasmtype(array(
                              'project_id' => $field_values[$fld_experiment],
                              'genus' => $field_values[$fld_genus],
                              'species' => $field_values[$fld_species],
                              'traits' => $field_values[$fld_traits],
                              'trait_method_unit_id' => $trait_method_unit_id,
                              'year' => $field_values[$fld_year],
                              'location' => $field_values[$fld_location]
                            ));

                            if ($options) {
                              $field_values[$fld_germplasmtype] = array_keys($options);

                              $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                              $field_prop[$fld_germplasmtype]['options'] = $options;
                              $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                            }
                          }
                        }

                        unset($options);

                        // Cache value selected.
                        $form['values'][ $fldname[$fld_year]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_year]);
                        $cache_values[$fld_year] = $form_state['values'][ $fldname[$fld_year]['cache'] ];

                        // AJAX - same experiment, genus, species, traits and year.
                        if ($cache_values[$fld_year] == implode('_', $field_values[$fld_year])) {

                          /////// Location selected - Create germplasm type options.
                          if ($form_state['values']['ap_' . $fld_location . '_field'] != '') {
                            $l = $form_state['values']['ap_' . $fld_location . '_field'];

                            // Many year, use all location. Otherwise, use what user checked.
                            if (count($field_values[$fld_year]) > 1) {
                              $field_values[$fld_location] = $field_values[$fld_location];
                            }
                            else {
                              $field_values[$fld_location] = analyzedphenotypes_checkboxesvalue($l);
                            }

                            if (count($field_values[$fld_location]) > 0) {
                              $options = ap_populate_fieldgermplasmtype(array(
                                'project_id' => $field_values[$fld_experiment],
                                'genus' => $field_values[$fld_genus],
                                'species' => $field_values[$fld_species],
                                'traits' => $field_values[$fld_traits],
                                'trait_method_unit_id' => $trait_method_unit_id,
                                'year' => $field_values[$fld_year],
                                'location' => $field_values[$fld_location]
                              ));

                              if ($options) {
                                $field_prop[$fld_germplasmtype]['options'] = $options;
                                $field_prop[$fld_germplasmtype]['disabled'] = FALSE;

                                // Germplasm Type.
                                if (count($field_values[$fld_location]) > 1) {
                                  // Many locations selected.
                                  $field_values[$fld_germplasmtype] = array_keys($options);

                                  $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                                  $options = ap_populate_fieldgermplasmtype(array(
                                    'project_id' => $field_values[$fld_experiment],
                                    'genus' => $field_values[$fld_genus],
                                    'species' => $field_values[$fld_species],
                                    'traits' => $field_values[$fld_traits],
                                    'trait_method_unit_id' => $trait_method_unit_id,
                                    'year' => $field_values[$fld_year],
                                    'location' => $field_values[$fld_location]
                                  ));

                                  if ($options) {
                                    $field_values[$fld_germplasmtype] = array_keys($options);

                                    $field_prop[$fld_germplasmtype]['default'] = $field_values[$fld_germplasmtype];
                                    $field_prop[$fld_germplasmtype]['options'] = $options;
                                    $field_prop[$fld_germplasmtype]['disabled'] = FALSE;
                                  }
                                }
                              }

                              // Cache value selected.
                              $form['values'][ $fldname[$fld_location]['cache'] ] = ap_cache_download_fieldvalue($field_values[$fld_location]);
                              $cache_values[$fld_location] = $form_state['values'][ $fldname[$fld_location]['cache'] ];

                              // AJAX - same experiment, genus, species, traits, year and location.
                              if ($cache_values[$fld_location] == implode('_', $field_values[$fld_location])) {

                                //////// Germplasm type selected - Update germplasm list.
                                if ($form_state['values'][ $fldname[$fld_germplasmtype]['field'] ] != '') {
                                  $g = $form_state['values'][ $fldname[$fld_germplasmtype]['field'] ];

                                  // Many year, use all location. Otherwise, use what user checked.
                                  if (count($field_values[$fld_location]) > 1) {
                                    $field_values[$fld_germplasmtype] = $field_values[$fld_germplasmtype];
                                  }
                                  else {
                                    $field_values[$fld_germplasmtype] = analyzedphenotypes_checkboxesvalue($g);
                                  }
                                ////////
                                }
                              // end AJAX - same experiment, genus, species, traits, year and location.
                              }
                              else {
                                if (isset($form_state['input'][ $fldname[$fld_germplasmtype]['field'] ])) {
                                  $input = $form_state['input'][ $fldname[$fld_germplasmtype]['field'] ];
                                  foreach($input as $i => $v) {
                                    if (!empty($v)) {
                                      unset($form_state['input'][ $fldname[$fld_germplasmtype]['field'] ]);
                                      break;
                                    }
                                  }
                                }
                              // End else ^
                              }
                            }
                          ///////
                          }
                        // end AJAX - same experiment, genus, species, traits and year.
                        }
                        else {
                          if (isset($form_state['input'][ $fldname[$fld_location]['field'] ])) {
                            $input = $form_state['input'][ $fldname[$fld_location]['field'] ];
                            foreach($input as $i => $v) {
                              if (!empty($v)) {
                                unset($form_state['input'][ $fldname[$fld_location]['field'] ]);
                                break;
                              }
                            }
                          }
                        // End else ^
                        }
                      }
                    //////
                    }
                  // end AJAX - same experiment, genus, species and traits.
                  }
                  else {
                    if (isset($form_state['input'][ $fldname[$fld_year]['field'] ])) {
                      $input = $form_state['input'][ $fldname[$fld_year]['field'] ];
                      foreach($input as $i => $v) {
                        if ($v > 0) {
                          unset($form_state['input'][ $fldname[$fld_year]['field'] ]);
                          break;
                        }
                      }
                    }
                  // End else ^
                  }
                }
              /////
              }
            // end AJAX - same experiment, genus and species.
            }
            else {
              if (isset($field_prop[$fld_traits]['default'][0]) && $field_prop[$fld_traits]['default'][0] == 0) {
                $reset_traits = 1;
              }
            }
          ///
          }
        // end AJAX - same experiment and genus.
        }
        else {
          // Reset species.
          $reset_species = 1;
        }

        // Update germplasm list.
        $values = array(
          'project_id' => $field_values[$fld_experiment],
          'genus' => $field_values[$fld_genus],
          'species' => $field_values[$fld_species],
          'traits' => $field_values[$fld_traits],
          'trait_method_unit_id' => $trait_method_unit_id,
          'year' => $field_values[$fld_year],
          'location' => $field_values[$fld_location],
          'germplasmtype' => $field_values[$fld_germplasmtype],
        );

        $options = ap_populate_fieldgermplasmlist($values);

        if ($options) {
          $field_prop[$fld_germplasm]['disabled'] = FALSE;
          $field_prop[$fld_germplasm]['options'] = $options;
          $field_prop[$fld_germplasm]['default'] = '';

          $field_prop[$fld_missingdata]['disabled'] = FALSE;
        }
      // END ONE EXPERIMENT.
      }

      // Replace Trait name to Traits selected.
      // eg. Plant Height (cm), Planting Data ....
      if (count($field_prop[$fld_traits]['options']) > 0) {
        $s = array();
        foreach($field_prop[$fld_traits]['options'] as $id => $name) {
          if (in_array($id, $field_values[$fld_traits])) {
            $s[] = $name;
          }
        }

        $traits_list = implode($delimiter, $s);

        $default_headers['Trait Name'] = $traits_list;
      }

      // Add Replicate to Default headers when user does not want to average data.
      if ($form_state['values'][ $fldname[$fld_averagerep]['field'] ] == 0) {
        $default_headers['Replicate'] = 'Replicate';
      }
    ///
    }
  //
  }


  //
  //
  // Fieldset: #1. About the traits.
  $ap_fieldset_1 = 'ap_about_trait_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_1] = array(
    '#type' => 'fieldset',
    '#title' => 'Select Trait.',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#id' => 'ap-about-traits-fieldset',
  );

    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_fieldset1_warning'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages warning" style="margin: 0 0 20px 0">
          Please note that filter criteria may become deactivated when exporting data from multiple experiments or species.
        </div>',
    );

    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_fieldset1_description'] = array(
      '#type' => 'item',
      '#description' => t('Indicate the trait you would like phenotypic data for by selecting
        experiment and the genus of the crop, as well as the name of the trait below. Field marked with a * means field is required and must have a value before proceeding.<br /><br /><strong>Restrict the dataset to a specific experiment</strong>. This can be done by clicking on the name of
        the experiment below. You can further filter by year and location if desired.'),
      '#id' => 'ap-fieldset1-description',
    );

    //
    // FIELD: EXPERIMENT:
    $exp = ap_populate_fieldproject();
    $field_prop[$fld_experiment]['options'] = ($exp) ? $exp : array();
    $experiment_count = count($field_prop[$fld_experiment]['options']);

    $form[$AJAX_wrapper][$ap_fieldset_1][$fld_experiment]['title'] = array(
      '#title' => t('Experiment'),
      '#type' => 'item',
      '#required' => TRUE,
    );

    $h = ($experiment_count < 2) ? 35 : 25;
    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_experiment]['field'] ] = array(
      '#type' => 'checkboxes',
      '#options' => $field_prop[$fld_experiment]['options'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_experiment]['field-id'],
    )
    + $AJAX_callback
    // Always reveal all available experiment.
    + analyzedphenotypes_wrapcheckbox($h * $experiment_count, $experiment_count);

    // Count data points, # of traits and # of stocks.
    $summary = ap_populate_fieldprojectsummary();

    // Show a short summary (# of data available) on mouse over.
    if ($summary) {
      $markup = '';

      foreach($summary as $id => $info) {
        $markup .= '<div id="ap-info-experiment' . $id . '" class="ap-tooltip">' . $info . '</div>';
      }

      $form[$AJAX_wrapper][$ap_fieldset_1][$fld_experiment]['tooltip'] = array(
        '#markup' => $markup,
      );
    }
    ///////


    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_genus_species_description'] = array(
      '#type' => 'item',
      '#description' => t('Select the genus of the crop you would like phenotypic data for. Additionally, the species
        can be indicated to further restrict the germplasm phenotypic data is exported for.'),

      '#id' => 'ap-genus-species-description',
    );


    //
    // FIELD: GENUS AND SPECIES:
    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_genus]['field'] ] = array(
      '#type' => 'select',
      '#title' => t('Genus'),
      '#required' => TRUE,
      '#default_value' => 0,
      '#empty_option'  => $field_prop[$fld_genus]['empty'],
      '#options'       => $field_prop[$fld_genus]['options'],
      '#disabled'      => $field_prop[$fld_genus]['disabled'],

      // Class to indicate field aligned next to another.
      '#prefix' => '<div class="ap-field-inline-container">',
      '#suffix' => '</div>',

      '#id' => $fldname[$fld_genus]['field-id'],
    )
    + $AJAX_callback;


    //
    // FIELD: SPECIES.
    $form[$AJAX_wrapper][$ap_fieldset_1][$fld_species]['title'] = array(
      '#title' => t('Species'),
      '#type' => 'item',
      '#required' => TRUE,
    );

    $form[$AJAX_wrapper][$ap_fieldset_1]['ap_' . $fld_species . '_field'] = array(
      '#type' => 'checkboxes',
      '#required' => TRUE,
      '#default_value' => isset($field_prop[$fld_species]['default']) ? $field_prop[$fld_species]['default'] : array(),
      '#options'       => $field_prop[$fld_species]['options'],
      '#disabled'      => $field_prop[$fld_species]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_species]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_species]['options']));

    // Select all options when many experiments selected.
    if (!isset($field_prop[$fld_species]['default'])) {
      $field_prop[$fld_species]['default'] = NULL; }
    if ($field_prop[$fld_species]['default'][0] > 0 || !empty($field_prop[$fld_species]['default'][0])) {
      foreach($field_prop[$fld_species]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_species]['field'] ][$o]['#attributes']
          = array('checked' => 'checked');
      }

      $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_species]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
    }

    $form[$AJAX_wrapper]['reset_species'] = array(
      '#type' => 'hidden',
      '#value' => (isset($reset_species)) ? 1 : 0,
      '#attributes' => array('id' => 'ap-reset-species'),
    );
    ///////


    //
    // FIELD: TRAIT NAME.
    $form[$AJAX_wrapper][$ap_fieldset_1][$fld_traits]['title'] = array(
      '#title' => t('Trait Name'),
      '#type' => 'item',
      '#required' => TRUE,
    );

    $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_traits]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_traits]['default'],
      '#options'       => $field_prop[$fld_traits]['options'],
      '#disabled'      => $field_prop[$fld_traits]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_traits]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(115, count($field_prop[$fld_traits]['options']));

    // Select all options when many experiments selected.
    if ($field_prop[$fld_traits]['default'][0] > 0 || !empty($field_prop[$fld_traits]['default'][0])) {
      foreach($field_prop[$fld_traits]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_traits]['field'] ][$o]['#attributes']
          = array('checked' => 'checked');
      }

      $form[$AJAX_wrapper][$ap_fieldset_1][ $fldname[$fld_traits]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
    }

    $form[$AJAX_wrapper]['reset_traits'] = array(
      '#type' => 'hidden',
      '#value' => (isset($reset_traits)) ? 1 : 0,
      '#attributes' => array('id' => 'ap-reset-traits'),
    );

    $form[$AJAX_wrapper]['ap_trait_method_unit_id'] = array(
      '#type' => 'hidden',
      '#value' => (isset($trait_method_unit_id)) ? implode('~', $trait_method_unit_id) : '',
      '#attributes' => array('id' => 'ap-trait-method-unit-id'),
    );
    ///////

  //
  //
  // Fieldset: #2. Additional filter criteria.
  $ap_fieldset_2 = 'ap_additional_filter_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_2] = array(
    '#title' => t('Additional filter criteria (Optional).'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

    $form[$AJAX_wrapper][$ap_fieldset_2]['ap_fieldset2_description'] = array(
      '#type' => 'item',
      '#description' => t('We recommend you fill out as many of the following filters as possible to narrow the phenotype set to those you are most interested in.'),
      '#id' => 'ap-fieldset2-description',
    );


    //
    // FIELD: YEAR.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_year]['title'] = array(
      '#title' => t('Year'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_year]['default'],
      '#options'       => $field_prop[$fld_year]['options'],
      '#disabled'      => $field_prop[$fld_year]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_year]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_year]['options']));

    // Select all options when many experiments selected.
    if ($field_prop[$fld_year]['default'][0] > 0 || !empty($field_prop[$fld_year]['default'][0])) {
      foreach($field_prop[$fld_year]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ][$o]['#attributes']
          = array('checked' => 'checked');
      }

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_year]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
    }


    //
    // FIELD: LOCATION.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_location]['title'] = array(
      '#title' => t('Location'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_location]['default'],
      '#options'       => $field_prop[$fld_location]['options'],
      '#disabled'      => $field_prop[$fld_location]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_location]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_location]['options']));

    // Select all options when many experiments selected.
    if ($field_prop[$fld_location]['default'][0] > 0 || !empty($field_prop[$fld_location]['default'][0])) {
      foreach($field_prop[$fld_location]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ][$o]['#attributes']
          = array('checked' => 'checked');
      }

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_location]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
    }


    //
    // FIELD: GERMPLASM TYPE.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_germplasmtype]['title'] = array(
      '#title' => t('Germplasm Type'),
      '#type' => 'item',
    );

    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ] = array(
      '#type' => 'checkboxes',
      '#default_value' => $field_prop[$fld_germplasmtype]['default'],
      '#options'       => $field_prop[$fld_germplasmtype]['options'],
      '#disabled'      => $field_prop[$fld_germplasmtype]['disabled'],

      // Class to indicate field is evaluated when generating a set of headers.
      '#attributes' => array('class' => array('ap-field-header')),
      '#id' => $fldname[$fld_germplasmtype]['field-id'],
    )
    + $AJAX_callback
    + analyzedphenotypes_wrapcheckbox(70, count($field_prop[$fld_germplasmtype]['options']));

    // Select all options when many experiments selected.
    if ($field_prop[$fld_germplasmtype]['default'][0] > 0 || !empty($field_prop[$fld_germplasmtype]['default'][0])) {
      foreach($field_prop[$fld_germplasmtype]['default'] as $o) {
        $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ][$o]['#attributes']
          = array('checked' => 'checked');
      }

      $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_germplasmtype]['field'] ]['#attributes']['class'][] = 'ap-no-mouse-event';
    }


    //
    // FIELD: GERMPLASM.
    $form[$AJAX_wrapper][$ap_fieldset_2][$fld_germplasm]['title'] = array(
      '#title' => t('Germplasm'),
      '#type' => 'item',
    );

    // GERMPLASM BAR
    // Suggest all germplasm found given the combination of fiters above.
    if ($field_prop[$fld_germplasm]['options']) {
      $germplasm_found = $field_prop[$fld_germplasm]['options'];

      // Indicate the number of germplasm matched given set of filters. Provide filter option to enter specific germplasm.
      $germplasm_text = '<strong>' . count($germplasm_found)  . ' germplasm</strong> found based on the filters above.';
      $germplasm_class = 'ap-field-enabled';
    }
    else {
      // Nothing found.
      $germplasm_found = array();
      $germplasm_text = '';
      $germplasm_class = 'ap-field-disabled';
    }

    // Germplasm/Stock Name control bar.
    $form[$AJAX_wrapper][$ap_fieldset_2]['ap_filter_germplasm_bar'] = array(
      '#markup' => '<div id="ap-filter-germplasm-bar" class="' . $germplasm_class . '">' . $germplasm_text . '</div>',
    );

    // Count germplasm names found.
    $germplasm_count = (isset($germplasm_found)) ? count($germplasm_found) : 0;

    if ($germplasm_count > 0) {
      // Define AJAX element wrapper.
      $field_container = 'ap_AJAX_germplasmname_field_wrapper';

      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container] = array(
        '#prefix' => '<div id="ap-AJAX-germplasmname-field-wrapper">',
        '#suffix' => '</div>',
        // Nest elements inserted, this important or else
        // everything will be wobbly.
        '#tree' => TRUE,
      );

      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $fldname[$fld_germplasm]['field'] ] = array(
        '#type' => 'checkboxes',
        '#options' => drupal_map_assoc($germplasm_found),
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_add_name',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#prefix' => '<div id="ap-filter-germplasm-found">',
        '#suffix' => '</div>',
        //
        '#id' => 'ap-field-germplasm-names',
      );

      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container]['title'] = array(
        '#markup' => '<div id="ap-filter-germplasm-reveal">
          <div class="ap-filter-germplasm-reveal-on" title="Show/Hide ' . count($germplasm_found) . ' Germplasm/Stock Name">&nbsp;</div>
          </div>

          <div id="ap-filter-germplasm-by-field">
            Phenotype for Specific Germplasm<br />
            <small>If you are interested in phenotypes for specific germplasm, you can add them individually by clicking add button or germplasm names.
            To retrive all germplasm based on your other filter criteria, proceed to the next filter.</small>
          </div>',
      );

      // Field names.
      $field = array(
        'field_hidden_values' => 'ap_field_hidden_values',
        'field_textfield'     => 'ap_field_textfield_', // suffix unique id.
        'field_button_add'    => 'ap_field_button_add',
        'field_button_rem'    => 'ap_field_button_rem_', // suffix unique id.
      );


      // Field array.
      $field_textfield = array(
        '#type' => 'textfield',
        '#title' => 'Type Germplasm Name/Stock Name',
        '#title_display' => 'invisible',
        '#size' => 65,
        '#ajax' => array(
          'event' => 'autocompletechange',
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_update_element',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('placeholder' => 'Type Germplasm Name/Stock Name', 'class' => array('ap-textfield-germplasmname')),
      );

      $field_button_rem = array(
        '#type' => 'button',
        '#value' => '',
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_remove_element',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('class' => array('remove-btn')),
      );

      // Each field inserted should contain a form of unique identifiers.
      $germplasm_field_id = uniqid();
      // Pass on to this variable each time a new field is added.
      $new_element = null;

      // Load values array (names pre-existing) by either buttons or
      // start wit a single empty field on initial page load.
      $values_array = (isset($form_state['triggering_element']))
        ? $form_state['values'][ $field['field_hidden_values']  ]
        : array($germplasm_field_id => '');

      // Append the one and only add button.
      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_button_add'] ] = array(
        '#type' => 'button',
        '#value' => '+',
        '#ajax' => array(
          'callback' => 'ap_AJAX_germplasmfield_callback',
          'command' => 'germplasm_add_blank',
          'wrapper' => 'ap-AJAX-germplasmname-field-wrapper',
          'progress' => array('type' => 'none'),
        ),
        '#attributes' => array('class' => array('add-btn')),
      );

      // Execute append action only when field count is less than the number
      // of germplasm and/or if added by link, germplasm is not previously added.

      if (isset($form_state['triggering_element'])) {
        // Action required and value.
        $triggering_element = array(
          'command' => $form_state['triggering_element']['#ajax']['command'],
          'value' => $form_state['triggering_element']['#value'],
        );

        // Execute command.
        // Save error - invalid/not found germplasm name.
        $error = array();

        // Flag if form has been submitted for the first time.
        // This will indicate the stage to add the first set of elements.
        $form['ap_form_submitted'] = array(
          '#type' => 'hidden',
          '#value' => isset($form_state['values']['ap_form_submitted']) ? TRUE : FALSE,
        );

        if ($triggering_element['command'] == 'germplasm_add_blank') {
          // ADD - add a blank field.
          // To add, check values array and inspect the number of empty
          // fields, if it exceeded 1, then don't allow any more blank fields.
          // Field is default to: Type Germplasm Name/Stock Name.
          $blanks = 0;

          foreach($values_array as $v) {
            if (empty($v)) {
              $blanks++;
              if ($blanks > 1) break;
            }
          }

          if ($blanks < 1 && count($germplasm_found) > 1) {
            // Can't add more than 1 empty field.
            $values_array[$germplasm_field_id] = '';
          }

          // See JS for autocomplete support in fields.
        }
        elseif ($triggering_element['command'] == 'germplasm_add_name') {
          // ADD - add field by checking a germplasm name.
          // To add, see if action required is to include or exclude a name.

          // Checking or uncheck-ing.
          if ($triggering_element['value'] === 0) {
            // To uncheck - Find the value (stock name) of the triggering element
            // to extract the unique id number then use it to reset the value in that key.
            $fid = array_search($form_state['triggering_element']['#return_value'], $values_array);
            unset($values_array[$fid]);

            // All fields got deleted leave one or add a single field.
            if (count($values_array) <= 0) {
              $values_array[$id] = '';
            }
          }
          else {
            // Tick off.
            // Add a field and set the value to matching germplasm name.
            $values_array[$germplasm_field_id] = $triggering_element['value'];
          }
        }
        elseif ($triggering_element['command'] == 'germplasm_update_element') {
          // ADD/REM - interacting with text field.
          // To add/rem, see if action required is to add or remove.
          $id =  $form_state['triggering_element']['#id'];
          $id_value = $form_state['input'][$field_container][ $field['field_textfield'] . $id ];

          if ($id_value) {
            // User typed in a germplasm name.
            // Double check that value entered matches a name in germplasm names.
            if (in_array($id_value, $germplasm_found)) {
              $values_array[$id] = $id_value;
            }
            else {
              // Bad name, does not exists.
              $error[$id] = '<small class="ap-germplasmfield-error">*The germplasm name/stock name you entered is invalid or does not exist.</small>';
            }
          }
          else {
            // User hit del/backspace key to remove germplasm name.
            // Cache the value removed, since this will be the only record of the
            // value and unchecking of name requres a reference to this value.
            $form['ap_cache_germplasmname'] = array(
              '#type' => 'hidden',
              '#value' => $values_array[$id],
            );

            // With value cached, we can leave out this item.
            $values_array[$id] = '';
          }
        }
        elseif ($triggering_element['command'] == 'germplasm_remove_element') {
          // REMOVE - hit x / remove button ajacent to the field.
          // Remove field.
          unset($values_array[ $triggering_element['value'] ]);

          // Uncheck matching germplasm name.
          // see callback.
        }
        else {
          // ADD - the first set of element that gets added.
          // A blank field and add button.
          if ($form_state['values']['ap_form_submitted'] == FALSE) {
            $values_array[$germplasm_field_id] = '';
            $form['ap_form_submitted']['#value'] = TRUE;
          }
        }
      }

      // Always leave one empty field.
      if (count($values_array) < 1) {
        $values_array[$id] = '';
      }

      // Render values array.
      foreach($values_array as $key => $value) {
        if (count($germplasm_found) == 1 && count($values_array) == 1 && empty($value)) {
          // When there is only a single germplasm found and adding it would add an extra field.
          unset($values_array[$key]);
        }
        elseif (count($values_array) >= count($germplasm_found) && empty($value)) {
          // When all names have been checked, remove extra blank field.
          unset($values_array[$key]);
        }
        elseif (!in_array($value, $germplasm_found)) {
          // Before officially adding an element to the DOM, make sure that
          // a name matches a germplasm in the list of germplasm found.
          $value = '';
        }

        // Text field.
        $field_textfield['#value'] = $value;
        $field_textfield['#id'] = $key;
        $field_textfield['#suffix'] = (isset($error[$key])) ? $error[$key] : '';
        $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_textfield'] . $key ] = $field_textfield;

        // Remove button.
        if (count($values_array) > 1) {
          $field_button_rem['#value'] = $key;
          $form[$AJAX_wrapper][$ap_fieldset_2][$field_container][ $field['field_button_rem'] . $key ] = $field_button_rem;
        }
      }

      // Record keeping.
      $form[ $field['field_hidden_values'] ] = array(
        '#type' => 'hidden',
        '#value' => $values_array,
      );

      // Construct suggestion array - in autocomplete field by excluding
      // name that had been checked.
      $suggest_germplasmname = $germplasm_found;
      foreach($values_array as $element_id => $element_value) {
        if (!empty($element_value)) {
          // A germplasm name in this field, remove it from suggestions.
          $o = array_search($element_value, $suggest_germplasmname);
          unset($suggest_germplasmname[$o]);
        }
      }

      // To autocomplete JS.
      $form[$AJAX_wrapper][$ap_fieldset_2][$field_container]['fld_suggest_germplasm'] = array(
        '#type' => 'hidden',
        '#value' => implode(',', $suggest_germplasmname),
        '#attributes' => array('id' => 'ap_field_suggest_germplasmnames'),

      );

    // End if germplasm_count.
    }

    /*
    // NOT IMPLEMENTED:
    // Commented out, this field does not have any effect on the exported data.
    // *Corresponding behaviours in js if this is enabled.
    //
    // FIELD: MAX MISSING DATA.
    $form[$AJAX_wrapper][$ap_fieldset_2][ $fldname[$fld_missingdata]['field'] ] = array(
      '#title' => t('Maximum Allowed Missing Data'),
      '#type' => 'textfield',
      '#default_value' => $field_prop[$fld_missingdata]['default'],
      '#disabled'      => $field_prop[$fld_missingdata]['disabled'],
      '#size' => 10,

      '#description' => t('Enter the percent (%) missing data per germplasm that you would like to allow. For example, a value of 20% will ensure that all germplasm exported have values for at least 20% of site-years this trait was observed in. If you further restrict the site-year exported using other filter criteria, this filter will be applied to the restricted dataset.'),
      '#id' => $fldname[$fld_missingdata]['field-id'],
    );
    */


  //
  //
  // Fieldset: #3. Choose your file.
  $ap_fieldset_3 = 'ap_file_type_fieldset';
  $form[$AJAX_wrapper][$ap_fieldset_3] = array(
    '#type' => 'fieldset',
    '#title' => 'Choose your output file.',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_fieldset3_description'] = array(
      '#type' => 'item',
      '#description' => t('Select the file format, column headers and summary options you would like the data exported in below.'),
      '#id' => 'ap-fieldset3-description',
    );


    // FILE TYPE:
    $download_type = ap_define_datafile_extensions('file_download');
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_filetype]['field'] ] = array(
      '#title' => t('File Type'),
      '#type' => 'select',
      '#disabled' => $field_prop[$fld_filetype]['disabled'],
      '#options'  => $download_type,
      '#description' => t('Select the format you would like the data exported.'),

      // Class to inidcate the field does not require AJAX.
      '#attributes' => array('class' => array('ap-skip-loading')),
      '#id' => $fldname[$fld_filetype]['field-id'],
    );

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_filetype_warning'] = array(
      '#type' => 'item',
      '#markup' => '<div>Please note that some germplasm names may have converted into dates in Microsoft Excel.</div>',
      '#id' => 'ap-filetype-warning',
    );


    //
    // INCLUDE ALL REPLICATES
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_averagerep ]['field'] ] = array(
      '#title' => t('Average Replicates per Site-Year'),
      '#type' => 'checkbox',
      '#default_value' => 1,
      '#disabled' => $field_prop[$fld_averagerep]['disabled'],
      '#description' => t('By default, all replicates in a single site-year will be averaged. Please specify if you would like all replicates to be included. To export the value of each replicate, uncheck this option.'),
      '#id' => $fldname[$fld_averagerep]['field-id'],
    )
    + $AJAX_callback;

    //
    // RFRIENDLY HEADERS.
    $form[$AJAX_wrapper][$ap_fieldset_3][ $fldname[$fld_rfriendly]['field'] ] = array(
      '#title' => t('Make Column Headers R Friendly'),
      '#type' => 'checkbox',
      '#disabled' => $field_prop[$fld_rfriendly]['disabled'],
      '#prefix' => '<div id="ap-chk-r">',
      '#suffix' => '</div>',

      // Class to inidcate the field does not require AJAX.
      '#attributes' => array('class' => array('ap-skip-loading')),
      '#id' => $fldname[$fld_rfriendly]['field-id'],
    );


    //
    // COLUMN HEADER PICKER.
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_title'] = array(
      '#type' => 'item',
      '#title' => 'Column Headers',
    );

    $header = array();
    $rows   = array();

    // HEADERS:

    // Default Headers.
    $i = 1;
    foreach($default_headers as $def) {
      $id = 'ap-default-row-' . $i;

      // Load the RFriendly version of headers.
      $def = str_replace($delimiter, ', ', $def);
      $rfriendly = ap_convert_rcolumnheader($def);
      // Comma separate trait list.
      $rfriendly = ucwords(str_replace(',.', ', ', $rfriendly));

      $rows[] = array(
        'number'   => array('title' => t('Order #'), 'id' => $id . 'seqn', 'class' => array('ap-seq-no'), 'data' => '#' . $i),
        'column'   => array('title' => t('Click to move one level up...'), 'id' => $id . 'cols', 'class' => array('ap-column'), 'data' => '<input type="checkbox" checked="checked" disabled="disabled"> <small>(default)</small> &nbsp;-  <span>' . $def . '</span><em>' . $rfriendly . '</em>'),
        'row-up'   => array('title' => t('Click to move one level up...'), 'id' => $id . 'goup', 'class' => array('ap-row-up'), 'data' => '&nbsp;'),
        'row-down' => array('title' => t('Click to move one level down...'), 'id' => $id . 'down', 'class' => array('ap-row-down'), 'data' => '&nbsp;')
      );

      $i++;
    }

    //
    // DEFAULT COLUMN HEADERS TABLE.
    $tbl1 = theme('table', array(
      'header' => $header,
      'rows'   => $rows,
      'attributes' => array('id' => 'ap-table-default-headers')
    ));

    // Preview Headers.
    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_default_headers'] = array(
      '#markup' => $tbl1
        . l('Preview headers', '#', array('attributes' => array('id' => 'ap-preview-link')))
        . '<div class="ap-clear-float"></div>',
    );


    //
    // PREIVEW COLUMN HEADERS TABLE.
    $tbl3 = theme('table', array(
      'header' => array(),
      'rows'   => array(),
      'attributes' => array('id' => 'ap-table-preview-headers'),
      'sticky' => FALSE,
    ));

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_preview_headers'] = array(
      '#markup' => '<div id="ap-table-preview-headers-container">
        <br />This is a sample data to show a preview of the otput file based on the column headers chosen above.'
        . $tbl3 . '<span>&nbsp;</span></div>',
    );

    //
    // OPTIONAL COLUMN HEADERS TABLE.
    $rows = array();
    $i = 1;

    foreach($optional_headers as $opt) {
      $id = 'ap-optional-row' . $i;

      // Load the RFriendly version of headers.
      $rfriendly = ap_convert_rcolumnheader($opt);

      $rows[] = array(
        'number'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'seqn', 'class' => array(), 'data' => '+'),
        'column'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'cols', 'class' => array('ap-column'), 'data' => '<input type="checkbox" id="' . $id  . 'checkbox" class="ap-optional-traits ap-skip-loading"> &nbsp;- <span>' . $opt . '</span><em>' . $rfriendly . '</em>'),
        'row-up'   => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'goup', 'class' => array(), 'data' => '&nbsp;'),
        'row-down' => array('title' => t('Check header to include/exclude in the output file.'), 'id' => $id . 'down', 'class' => array(), 'data' => '&nbsp;')
      );

      $i++;
    }

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_optional_table_title'] = array(
      '#type' => 'item',
      '#title' => 'Optional Headers',
    );

    $tbl2 = theme('table', array(
      'header' => $header,
      'rows'   => $rows,
      'attributes' => array('id' => 'ap-table-optional-headers')
    ));

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_optional_headers'] = array(
      '#markup' => $tbl2,
    );

    $form[$AJAX_wrapper][$ap_fieldset_3]['ap_column_header_picker_container']['ap_table_description'] = array(
      '#type' => 'item',
      '#description' => t('Check the column headers you want to include in the output file. Use the up and down arrow buttons to change the order they appear in the file. Click Preview headers to preview selected column headers.'),
    );


    //
    // STORE VALUES OF COLUMN HEADERS.
    $headers = implode($delimiter, $default_headers);

    $form[$AJAX_wrapper][ $fldname[$fld_columnheaders]['field'] ] = array(
     '#type' => 'hidden',
     '#value' => $headers,
     '#attributes' => array('id' => $fldname[$fld_columnheaders]['field-id']),
    );


  //
  // SUBMIT BUTTON.
  drupal_add_js(array('analyzedphenotypes' => array('btn' => $btn_ok)), array('type' => 'setting'));

  $form['ap_download_submit'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#weight' => 100,

    // Disabled by default.
    '#attributes' => array(
      'class' => array('form-button-disabled'),
      'disabled' => 'disabled',
    ),
    '#id' => 'ap-download-submit-field',
  );


  return $form;
}


/**
 * Implements hook_submit().
 */
function analyzedphenotypes_downloaddata_form_submit($form, &$form_state) {
  // Use this character to separate entries in a string.
  $delimiter = '~';
  $field_values = array();

  // Field names.
  $fldname = ap_construct_download_fieldnames();

  foreach($fldname as $f) {
    if (isset($form_state['values'][ $f['field'] ]) ||
      isset($form_state['values']['ap_AJAX_germplasmname_field_wrapper'][ $f['field'] ])) {
      // NOTE: germplasm name had #tree option - check value with this option on.

      $val = (isset($form_state['values'][ $f['field'] ]))
        ? $form_state['values'][ $f['field'] ]
        : $form_state['values']['ap_AJAX_germplasmname_field_wrapper'][ $f['field'] ];

      if (is_array($val)) {
        // Field checkboxes.
        $v = analyzedphenotypes_checkboxesvalue($val);
        $entered_value = ($v) ? implode($delimiter, $v) : 'all';
      }
      else {
        $entered_value = trim(strip_tags($val));
      }

      $field_values[ $f['base'] ] = $f['base'] . '=' . $entered_value;
    }
  }

  // Lastly, extract trait ids.
  // Choices
  $ids = $form_state['values']['ap_trait_method_unit_id'];
  $tmp = explode($delimiter, $ids);
  $trait_method_unit_id = array_combine(range(1, count($tmp)), array_values($tmp));
  // Chosen
  $trait_field = $form_state['values']['ap_traits_field'];
  $selected = analyzedphenotypes_checkboxesvalue($trait_field);

  // Id of chosen.
  $traits = ap_extract_trait_method_unit_id($selected, $trait_method_unit_id);
  $field_values['traits'] = 'traits=' . implode($delimiter, $traits);

  // Prepare query string.
  $querystring = implode('&', $field_values);
  $url = base64_encode($querystring);

  $form_state['redirect'] = array(
    '/phenotype/download/analyzed_phenotypic_data',
    array(
      'query' => array('code' => $url),
    ),
  );
}


// HELPER FUNCTIONS:

/**
 * Function wrap a checkbox to a div and stlye it to look like a multi select box.
 *
 * @param $height
 *   An integer, height value of the container element.
 * @param $has_options
 *   An integer, the number of options available.
 */
function analyzedphenotypes_wrapcheckbox($height, $has_options) {
  // Set field to disabled when checkbox has no options available.
  $class = ($has_options > 0) ? '' : ' ap-field-disabled';


  return array(
    '#prefix' => '<div class="ap-checkboxes-container' . $class . '" style="height: '. $height .'px">
                    <div class="ap-checkbox-wrapper">',
    '#suffix' => '  </div>
                  </div>',
  );
}


/**
 * Function get checked values from field checkboxes.
 *
 * @param $value
 *   An array of values returned when check a checkbox.
 */
function analyzedphenotypes_checkboxesvalue($value) {
  $checkboxes_value = array();

  if ($value) {
    // Sort values selected first to take into account
    // the order options are selected/checked.
    asort($value);

    foreach($value as $v) {
      if ($v > 0 || $v != '') {
        $checkboxes_value[] = $v;
      }
    }
  }


  return $checkboxes_value;
}
