<?php
/**
 *
 * @see TripalImporter
 */
class TraitImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Phenotypic Trait Importer';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'ap_trait_importer';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Loads Traits for phenotypic data into the system. This is useful for large phenotypic datasets to ease the upload process.';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['txt', 'tst', 'csv'];

  /**
   * If the loader should require an analysis record.  To maintain provenance
   * we should always indiate where the data we are uploading comes from.
   * The method that Tripal attempts to use for this by associating upload files
   * with an analysis record.  The analysis record provides the details for
   * how the file was created or obtained. Set this to FALSE if the loader
   * should not require an analysis when loading. if $use_analysis is set to
   * true then the form values will have an 'analysis_id' key in the $form_state
   * array on submitted forms.
   */
  public static $use_analysis = FALSE;

  /**
   * If the $use_analysis value is set above then this value indicates if the
   * analysis should be required.
   */
  public static $require_analysis = FALSE;

  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'This should be a tab-separated file
    with the following columns: <ol>
      <li><strong>Trait Name:</strong> the name of the trait, as you would like it to appear
          to the user (e.g. Days to Flower).</li>
      <li><strong>Trait Description:</strong> a full description of the trait. This is
          recommended to be at least one paragraph.</li>
      <li><strong>Method Short Name:</strong> A full, unique title for the method (e.g. Days
          till 10% of plants/plot have flowers)</li>
      <li><strong>Collection Method:</strong> A full description of how the trait was collected.
          This is also recommended to be at least one paragraph.</li>
      <li><strong>Unit:</strong> The full name of the unit used (e.g. days, centimeters)</li>
      <li><strong>Type:</strong> One of "Qualitative" or "Quantitative".</li>
    </ol>
    <p>NOTE: The order of the above columns is important and your file <strong>must include a header</strong>!
       If you have a single trait measured in more then one way (i.e. with multiple collection
       methods), then you should have one line per collection method with the trait repeated.</p>';

  /**
   * Indicates the methods that the file uploader will support.
   */
  public static $methods = [
    // Allow the user to upload a file to the server.
    'file_upload' => TRUE,
    // Allow the user to provide the path on the Tripal server for the file.
    'file_local' => TRUE,
    // Allow the user to provide a remote URL for the file.
    'file_remote' => TRUE,
  ];

  /**
   * @see TripalImporter:form().
   */
  public function form($form, &$form_state) {

    $description = "This importer allows for the upload of phenotypic trait dictionaries in preparation for uploading phenotypic data. <br /><strong>This importer Does NOT upload phenotypic measurements.</strong>";
    $form['description'] = array(
      '#type' => 'markup',
      '#markup' => '<p>'.$description.'</p>',
      '#weight' => -100,
    );

    // Retrieve all configured genus'.
    $genus_options = array();
    $genusprop = ap_get_activegenus();
    if ($genusprop) {
      foreach($genusprop as $g) {
	$genus_options[$g] = $g;
      }
    }

    // If there is only one genus, it should be the default.
    $default_genus = 0;
    if (sizeof($genus_options) == 1) {
      $default_genus = reset($genus_options);
    }

    // Finally, add the genus select list.
    $form['genus'] = array(
      '#type' => 'select',
      '#title' => 'Genus',
      '#description' => 'The genus of the germplasm being phenotyped with the supplied traits. Traits in this system are specific to the genus in order to ensure they are specific enough to accuratly describe the phenotypes. In order for genus to be availabe here is must be first configured in the Analyzed Phenotypes configuration.',
      '#empty_option' => '- Select -',
      '#options' => $genus_options,
      '#default_value' => $default_genus,
      '#weight' => -99,
    );

    return $form;
  }

  /**
   * @see TripalImporter:formValidate().
   */
  public function formValidate($form, &$form_state) {
    // No need to validate since Drupal automatically validates select lists.
  }

  /**
   * @see TripalImporter::run()
   */
  public function run() {

    // All values provided by the user in the Importer's form widgets are
    // made available to us here by the Class' arguments member variable.
    $arguments = $this->arguments['run_args'];

    // The only arguement we need is the genus. This is used to determine
    // which cv's to insert the terms into based on configuration
    // handled by this module.
    $genus = $arguments['genus'];

    // The path to the uploaded file is always made available using the
    // 'files' argument. The importer can support multiple files, therefore
    // this is an array of files, where each has a 'file_path' key specifying
    // where the file is located on the server.
    $file_path = $this->arguments['files'][0]['file_path'];

    $FILE = fopen($file_path, 'r');
    $linenum = 0;
    while (($data = fgetcsv($FILE, 100000, "\t")) !== FALSE) {
      if ($linenum !== 0) {
        // Now simply insert the trait!
        $info = ap_insert_trait(array(
          'genus' => $genus,
          'name' => $data[0],
          'description' => $data[1],
          'method_title' => $data[2],
          'method' => $data[3],
          'unit' => $data[4],
          'type' => $data[5],
        ));
      }
      $linenum++;
    }
    fclose($FILE);
  }
}
