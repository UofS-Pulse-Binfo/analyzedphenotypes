Uploading of Analyzed Phenotypic Data
=====================================

Upload File Format
-------------------

The upload data page handles and processes analyzed phenotypic data provided in Tab-Delimited Values or .TSV format. This file is expected to contain the following columns:

- **Trait Name**: The full name of the trait as you would like it to appear on a trait page. This should not be abbreviated. (e.g. Days till one open flower)
- **Method Name**: A short (<4 words) name describing the method. This should uniquely identify the method while being very succinct. (e.g. 10% Plot at R1)
- **Unit**: The unit the trait was measured with. In the case of a scale this column should defined the scale. (e.g. days)
- **Germplasm Accession**: The stock.uniquename for the germplasm whose phenotype was measured. (e.g. ID:1234)
- **Germplasm Name**: The stock.name for the germplasm whose phenotype was measured. (e.g. Variety ABC)
- **Year**: The 4-digit year in which the measurement was taken. (e.g. 2020)
- **Location**: The full name of the location either using "location name, country" or GPS coordinates (e.g. Saskatoon, Canada)
- **Replicate**: The number for the replicate the current measurement is in. (e.g. 3)
- **Value**: The measured phenotypic value. (e.g. 34)
- **Data Collector**: The name of the person or organization which measured the phenotype.

The following is a short example:

.. code-block:: none

  Trait Name	Method Name	Unit	Germplasm Accession	Germplasm Name	Year	Location	Replicate	Value	Data Collector
  Lorem ipsum	dolor sit amet	metris	ID:1	GERM1	2015	"Neither up, nor Down"	1	5.3	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:2	GERM2	2015	"Neither up, nor Down"	1	2.2	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:3	GERM3	2015	"Neither up, nor Down"	1	4.9	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:1	GERM1	2015	There	1	5.1	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:2	GERM2	2015	There	1	3.6	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:3	GERM3	2015	There	1	4	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:1	GERM1	2015	Here	1	5.1	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:2	GERM2	2015	Here	1	3.3	Lacey Sanderson
  Lorem ipsum	dolor sit amet	metris	ID:3	GERM3	2015	Here	1	4.5	Lacey Sanderson

You can see a full example of this file distributed with the module: ``tests/example_files/AnalyzedPhenotypes-TestData-1trait3loc2yr3rep.txt``.

Upload Process
---------------

The upload process is divided into 4 stages.

.. image:: loading.1.upload-stage-indicators.png

Figure 1: Stage Indicator in Upload Page.

Stage 1: Upload
^^^^^^^^^^^^^^^^^

Basic compliance tests on the file level are performed to ensure that requirements outlined are met. For instance, file must be a valid .tsv file and experiment has been selected.

.. image:: loading.2.stage-1.png

.. image:: loading.3.drag-and-drop-feature.png

Figure 2: Upload Page Supports Drag and Drop File Upload, as well as Manual Upload.

Stage 2: Validate
^^^^^^^^^^^^^^^^^^

In this stage, the file undergoes a data level validation where data in rows and columns are tested against a set of validation rules to ensure that a value meets a set of conditions and requirements.

.. image:: loading.4.stage-2.png

Figure 3: Validate Stage shows progress bar to show user the status of data validation process.

Stage 3: Describe
^^^^^^^^^^^^^^^^^^

The file is further examined for all the unique traits in the Trait Name column. Information is then requested from the user for each trait detected.

.. image:: loading.5.stage-3.png

Figure 4: Describe Stage showing data form requesting user to provide information about the trait detected in data file.

Stage 4: Save
^^^^^^^^^^^^^^

File and data are stored.

.. image:: loading.6.stage-4.png

Figure 5: Show the final stage of the upload process. Similar to validate stage, a progress bar shows the status of saving process.
