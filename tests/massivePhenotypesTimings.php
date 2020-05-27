<?php
/**
 * Purpose: Run timings on the current database.
 * Total Time: at least 4 hours
 * Replicates: 9
 * Time between Replicates: 20 min
 *
 * RUN USING DRUSH (from the current directory):
 *   drush php-script tests/massivePhenotypesTimings.php
 */

$trait_id = NULL;
$project_id = NULL;
$number_of_reps = 9;
$time_btw_reps = 20 * 60; // in seconds.


if (!is_numeric($trait_id)) {
  print "Please edit the script to add in an existing trait identifier. This should be the attr_id from the chado.phenotype table.\n";
  exit();
}

// --------------------------------------------------------
// QUERY DEFINITION

// This is the query executed to extract the quantitative data
// collected for a single trait within a single experiment.
$quant_mview = 'SELECT location, year, stock_name, values
  FROM chado.mview_phenotype
  WHERE experiment_id=:project_id AND trait_id=:trait_id';
$quant_raw = "SELECT
  loc.value as location,
  yr.value as year,
  s.name as stock_name,
  array_to_json(array_agg(p.value)) AS values
FROM chado.phenotype p
  LEFT JOIN chado.cvterm trait ON trait.cvterm_id=p.attr_id
  LEFT JOIN chado.project proj USING(project_id)
  LEFT JOIN chado.stock s USING(stock_id)
  LEFT JOIN chado.organism o ON o.organism_id=s.organism_id
  LEFT JOIN chado.phenotypeprop loc ON loc.phenotype_id=p.phenotype_id
    AND loc.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Location')
  LEFT JOIN chado.phenotypeprop yr ON yr.phenotype_id=p.phenotype_id
    AND yr.type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='Year')
  WHERE trait.cvterm_id = :trait_id AND proj.project_id = :project_id
GROUP BY trait.cvterm_id, trait.name, proj.project_id, proj.name, loc.value, yr.value, s.stock_id, s.name, o.genus";

// This query is executed to summarize the number of genotypes per genus.
$summary_mview = "SELECT * FROM chado.mview_phenotype_summary";
$summary_raw = "(SELECT organism_genus, 1 as num, 1 as count
       FROM chado.mview_phenotype
       GROUP BY organism_genus)
    UNION
      (SELECT organism_genus, 2 as num, count(1)
       FROM (SELECT DISTINCT organism_genus, trait_id FROM chado.mview_phenotype) d2
       GROUP BY organism_genus)
    UNION
      (SELECT organism_genus, 3 as num, count(1)
       FROM (SELECT DISTINCT organism_genus, experiment_id FROM chado.mview_phenotype) d3
       GROUP BY organism_genus)
    UNION
      (SELECT organism_genus, 4 as num, count(1)
       FROM (SELECT DISTINCT organism_genus, stock_id FROM chado.mview_phenotype) d4
       GROUP BY organism_genus)
    UNION
      (SELECT organism_genus, 5 as num, count(1)
       FROM chado.mview_phenotype
       GROUP BY organism_genus)";

// --------------------------------------------------------
// TIMING EXECUTION

for ($i=0; $i < $number_of_reps; $i++) {

  print "\n\n" . str_repeat("=", 60)
    . "\n\tTIMING: " . ($i+1) . "\n"
    . str_repeat("=", 60) . "\n\n";

  print "\nQuantitative Query (via mview):\n" . str_repeat("-", 60) . "\n";
  $result = chado_query('EXPLAIN ANALYZE ' . $quant_mview,
    [':trait_id' => $trait_id, ':project_id' => $project_id])->fetchAll();
  foreach ($result as $r) {
    print $r->{'QUERY PLAN'} . "\n";
  }

  print "\nQuantitative Query (without mview):\n" . str_repeat("-", 60) . "\n";
  $result = chado_query('EXPLAIN ANALYZE ' . $quant_raw,
    [':trait_id' => $trait_id, ':project_id' => $project_id]);
  foreach ($result as $r) {
    print $r->{'QUERY PLAN'} . "\n";
  }

  print "\nSummary Query (via mview):\n" . str_repeat("-", 60) . "\n";
  $result = chado_query('EXPLAIN ANALYZE ' . $summary_mview);
  foreach ($result as $r) {
    print $r->{'QUERY PLAN'} . "\n";
  }

  print "\nSummary Query (without mview):\n" . str_repeat("-", 60) . "\n";
  $result = chado_query('EXPLAIN ANALYZE ' . $summary_raw);
  foreach ($result as $r) {
    print  $r->{'QUERY PLAN'} . "\n";
  }

  if ($number_of_reps > 1) {
    print "\n...SLEEPING FOR $time_btw_reps seconds between replicates...\n";
    sleep($time_btw_reps);
  }
}

print "\n\n" . str_repeat("=", 60) . "\nTIMINGS COMPLETE.\n\n";
