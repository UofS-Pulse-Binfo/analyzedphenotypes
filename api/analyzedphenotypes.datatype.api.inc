<?php
/**
 * @file
 *
 * API: UNIT AND DATA TYPES
 * Manage property and operation pertaining to trait unit and data type.
 */



/**
 * DATA SOURCE
 * Define data type, name and symbol of a unit.
 *
 * @return array.
 *   An array containing the name, unit symbol and the data type. Data types:
 *   - alphanumeric    : Letters, number combination.
 *   - trait           : String value strickly in - Trait name (unit) additional text - format.
 *   - text            : Letters only.
 *   - number          : Numbers + 0.
 *   - number_no_zero  : Numbers > 0.
 *   - scale_1-5       : 1,2,3,4 and 5.
 *   - scale_1-10      : 1 - 10.
 *   - y/n/?           : y, yes, n, no, unsure, ?.
 *   - date            : YYYY-MM-DD.
 *   - year            : YYYY
 *   - days            : Number of days to.
 */
function ap_define_datatypes() {
  $datatype = array(
    'g' => array('name' => 'grams',    'type' => 'number'),
    'kg' => array('name' => 'kilograms',  'type' => 'number'),
    'cm'  => array('name' => 'centimeters', 'type' => 'number'),
    'mg'   => array('name' => 'milligrams',   'type' => 'number'),
    'mM'    => array('name' => 'milimolar',  'type' => 'number'),
    'days'   => array('name' => 'days',     'type' => 'number_no_zero'),
    'date'     => array('name' => 'date', 'type' => 'date'),
    'text'      => array('name' => 'text', 'type' => 'alphanumeric'),
    'year'       => array('name' => 'year', 'type' => 'four_digit_year'),
    'count'       => array('name' => 'count', 'type' => 'number'),
    'y/n/?'        => array('name' => 'Yes (y), No (n) or Unsure (?)', 'type' => 'y/n/?'),
    'trait'         => array('name' => 'trait',   'type' => 'trait'),
    'colour'         => array('name' => 'color',     'type' => 'text'),
    'percent'         => array('name' => 'percentage',  'type' => 'number'),
    'comments'         => array('name' => 'comments',     'type' => 'text'),
    'scale 1-5'         => array('name' => 'scale 1-5',     'type' => 'scale_1-5'),
    'scale 1-10'         => array('name' => 'scale 1-10',     'type' => 'scale_1-10'),
    'average count'       => array('name' => 'average count',   'type' => 'number'),

    // Define unit here...
  );


  return $datatype;
}


/**
 * FUNCTION:
 * Fetch the data type of a unit. The data type will be used to validate value/data to ensure
 * that data matches the unit the trait is measuring.
 * Dependencies: ap_define_datatype().
 *
 * @param $unit
 *   A string value, the unit of a trait.
 *
 * @return
 *   String, data type of unit.
 */
function ap_get_datatype($unit) {
  // Set the expected data type of unit.
  $datatype = ap_define_datatypes();

  // Could not resolve unit, use alphanumeric data type TEXT.
  return (isset($datatype[ $unit ])) ? $datatype[ $unit ]['type'] : $datatype['text']['type'];
}


/**
 * FUNCTION:
 * Test data or value entered to see if it matches the data type the unit is expecting.
 * Dependencies: ap_get_datatype(),
 *
 * @param $unit
 *   String, unit of a trait.
 * @param $value
 *   Value or data entered.
 *
 * @return
 *   Bolean TRUE or FALSE - A false value means that value entered does
 *   not match the unit.
 */
function ap_validate_unitvalue($unit, $value) {
  $is_valid = TRUE;

  // Fetch data type of unit.
  $datatype = ap_get_datatype($unit);

  switch($datatype) {
    //
    case 'alphanumeric':
      // Letters, numbers and any 0 or more characters.
      if (preg_match('/[a-z0-9.*]/i', $value) !== 1) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'date':
      // YYYY-MM-DD format.
      if (preg_match('/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $value, $matches)) {
        $year  = $matches[1];
        $month = $matches[2];
        $day   = $matches[3];

        if ($month >= 1 AND $month <= 12 AND $day >= 1 AND $day <= 31 AND $year > 1900) {
          $today = new DateTime();
          $date = DateTime::createFromFormat('Y-m-d', $value);

          if ($date <= $today) {
            $is_valid = FALSE;
          }
        }
      }

      break;

    //
    case 'four_digit_year':
      // Present years and in the past but not beyond 1900.
      if ($value < 1900 && $value > date('Y')) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'number':
      // Numbers including 0.
      if (!is_numeric($value)) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'number_no_zero':
      // Numbers, no 0.
      if (preg_match('/[1-9]/', $value) !== 1) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'trait':
      // Alphanumeric chars followed by ( letters and ) - Trait name (unit).
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $value) !== 1) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'text':
      // Letters.
      if (preg_match('/[a-z]/i', $value) !== 1) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'scale_1-5':
      // 1, 2, 3, 4, 5.
      $data = (int)$value;
      $min = 1;
      $max = 5;

      if ($value < $min || $value > $max) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'scale_1-10':
      // 1, 2, 3, 4, ... 10.
      $data = (int)$value;
      $min = 1;
      $max = 10;

      if ($value < $min || $value > $max) {
        $is_valid = FALSE;
      }

      break;

    //
    case 'y/n/?':
      // Y, N, ?.
      $enum = array('y', 'yes', 'n', 'no', 'unsure', '?');
      if (!in_array($value, $enum)) {
        $is_valid = FALSE;
      }

      break;


    // Define more data type here...
  }

  return $is_valid;
}
