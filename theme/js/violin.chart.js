(function($) {
  Drupal.behaviors.analyzedPhenotypesViolinPlot = {
    attach: function (context, settings) {

    Drupal.settings.analyzedPhenotypes.forEach(function(apSettings) {

      // Draw the chart.
      var chart;
      var experiment_id = apSettings.experiment_id;
      var trait_id = apSettings.trait_id;
      var method_id = apSettings.method_id;
      var unit_id = apSettings.unit_id;
      var elementID = apSettings.id;

      if (apSettings.type === 'violin') {
        d3.json(Drupal.settings.basePath+'/json/phenotypes/traitplot/'+experiment_id+'/'+trait_id+'/'+method_id+'/'+unit_id, function(error, data) {

		console.log(data);
          d3.selectAll('#'+elementID+' .inner-wrapper').remove();

         // Ensure the datapoint is a number.
         data.forEach(function (d) {d.value = +d.value;});

         // Draw the x/yaxis of the chart and label.
         chart = makeDistroChart({
                  data:data,
                  xName:'category',
                  yName:'value',
                  axisLabels: {
                    xAxis: Drupal.settings.analyzedPhenotypes.xaxis,
                    yAxis: Drupal.settings.analyzedPhenotypes.yaxis
                  },
                  selector:"#"+elementID,
                  margin:{top: 15, right: 15, bottom: 40, left: 55},
                  chartSize:{height:1300, width:2000},
                  constrainExtremes:false});

          // Render the plot.
          chart.renderBoxPlot();
          chart.renderViolinPlot({
            clamp:0,
            colors:['#314355']
          });
          chart.boxPlots.show({
            reset:true,
            showWhiskers:true,
            showOutliers:false,
            boxWidth:10,
            lineWidth:15,
            colors:['#555']
          });

        }); //end of get json.
      }
    });
  }}; // End of Drupal Behaviours and associated attach.
})(jQuery);
