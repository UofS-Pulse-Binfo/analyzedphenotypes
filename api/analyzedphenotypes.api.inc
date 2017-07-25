<?php

/**
 *
 * This user-land implementation follows the implementation quite strictly;
 * it does not attempt to improve the code or algorithm in any way. It will
 * raise a warning if you have fewer than 2 values in your array, just like
 * the extension does (although as an E_USER_WARNING, not E_WARNING).
 *
 * @param array $a
 * @param bool $sample [optional] Defaults to false
 * @return float|bool The standard deviation or false on error.
 */
function analyzedphenotypes_stats_standard_deviation(array $a, $sample = false) {
  $n = count($a);

  if ($n === 0) {
      return false;
  }

  if ($sample && $n === 1) {
      return false;
  }

  $mean = array_sum($a) / $n;
  $carry = 0.0;

  foreach ($a as $val) {
    $d = ((double) $val) - $mean;
    $carry += $d * $d;
  };

  if ($sample) {
     --$n;
  }

  return sqrt($carry / $n);
}

/**
 *
 */
function analyzedphenotypes_table_values($rows, $column_row, $trait) {
  $columns = analyzedphenotypes_columnsprop('expected');

  $index_trait = array_search($columns[0], $column_row);
  $index_value = array_search($columns[6], $column_row);
  $index_year  = array_search($columns[3], $column_row);
  $index_site  = array_search($columns[4], $column_row);

  $arr_siteyear = array();
  $arr_values = array();

  foreach($rows as $i => $line) {
    if ($i > 0 && !empty($line)) {
      $data = str_getcsv($line, "\t");
      $data = array_map('trim', $data);

      if ($data[$index_trait] == $trait) {
        $value = $data[$index_value];
        $is_number = analyzedphenotypes_datatypeprop('number', $value);

        if ($is_number) {
          $site_year = $data[$index_site] . ' ' . $data[$index_year];

          if (in_array($site_year, $arr_siteyear)) {
            $arr_values[$site_year][] = $value;
          }
          else {
            $arr_siteyear[] = $site_year;
            $arr_values[$site_year][] = $value;
          }
        }
        else {
          return 0;
        }

      }
    }
  }


  sort($arr_siteyear);
  return array('siteyear' => $arr_siteyear, 'values' => $arr_values);
}
