TARGET=report
TEX=xelatex -shell-escape
BIBTEX=bibtex
READER=evince

all: view

rebuild output/$(TARGET).pdf: *.tex *.bib output
	cd output && rm -f *.tex *.bib && ln -s ../*.tex ../*.bib .
	cd output && $(TEX) $(TARGET).tex && $(BIBTEX) $(TARGET).aux && \
		$(TEX) $(TARGET).tex

output: 
	mkdir output
	cd output && rm -f res && ln -s ../res .

view: output/$(TARGET).pdf
	$(READER) $< &

clean:
	rm -rf output
	rm $(TARGET).pdf

# compile twice to get cross reference right
final : *.tex *.bib output
	cd output && rm -f *.tex *.bib && ln -s ../*.tex ../*.bib .
	cd output && $(TEX) $(TARGET).tex && $(BIBTEX) $(TARGET).aux && \
		$(TEX) $(TARGET).tex && $(TEX) $(TARGET).tex
	ps2pdf14 output/$(TARGET).pdf ./$(TARGET).pdf
	$(READER) ./$(TARGET).pdf

count:
	@word_count *.tex

dist: output/$(TARGET).pdf
	rm -rf paper
	mkdir paper
	cp output/$(TARGET).pdf paper/
	cp -r screenshots paper/
	7z a -tzip paper.zip paper
	rm -rf paper

.PHONY: all view clean rebuild dist
