This module is designed to provide support for partially analyzed phenotypic data. Specifically, the data should be filtered for outliers, etc. before being loaded into the system. The system will then handle averaging of replicates before display.

## Features
This module intends to provide the following features although it's currently in active development.
- Specialized upload form for end users to submit filtered phenotypic data.
- Trait pages for each trait data has been submitted for. These pages will 
   - describe the trait, 
   - link to ontology terms, 
   - define the method and units used to collect the data,
   - list experiments and site years with data for this trait,
   - summarize observed values per experiment in a bean chart
   - list mean/min/max for each germplasm, site year, experiment combination
- Summarize phenotypes on Germplasm pages.
- Summarize phenotypes on Project pages.
- Provide an interactive map showing all sites phenotypic data was recorded in that links to download for those sites.
- Provide a download form to allow users to pull down slices of phenotypic data: filter by location, year, trait, germplasm, project, etc.
