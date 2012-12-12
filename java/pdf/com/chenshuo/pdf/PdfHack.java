package com.chenshuo.pdf;

import java.io.FileOutputStream;

import com.itextpdf.text.Document;
import com.itextpdf.text.PageSize;
import com.itextpdf.text.Rectangle;
import com.itextpdf.text.pdf.PdfContentByte;
import com.itextpdf.text.pdf.PdfDictionary;
import com.itextpdf.text.pdf.PdfImportedPage;
import com.itextpdf.text.pdf.PdfName;
import com.itextpdf.text.pdf.PdfReader;
import com.itextpdf.text.pdf.PdfRectangle;
import com.itextpdf.text.pdf.PdfStamper;
import com.itextpdf.text.pdf.PdfWriter;

public class PdfHack {

    static final float kTextWidth = Float.parseFloat(System.getProperty("TextWidth", "370"));
    static final float kTextHeight = Float.parseFloat(System.getProperty("TextWidth", "590"));
    static final float kMargin = Float.parseFloat(System.getProperty("Margin", "10"));
    static final float kOffset = Float.parseFloat(System.getProperty("Offset", "0"));

    private static void help() {
        System.out.println("Usage: PdfHack command input_pdf");
        System.out.println("command is one of 'crop', 'twoup', 'booklet'");
    }

    private static void crop(String input) throws Exception {
        String output = input.replace(".pdf", "-crop.pdf");
        PdfReader reader = new PdfReader(input);
        final int n = reader.getNumberOfPages();
        Rectangle pageSize = reader.getPageSize(1);

        System.out.println("Input page size: " + pageSize);
        float left = (pageSize.getWidth() - kTextWidth) / 2 - kMargin;
        float right = pageSize.getWidth() - left;
        float bottom = (pageSize.getHeight() - kTextHeight) / 2;
        float top = pageSize.getHeight() - bottom;
        PdfRectangle rect = new PdfRectangle(left, bottom + kOffset, right, top + kOffset);
        for (int i = 1; i <= n; i++) {
            PdfDictionary pageDict = reader.getPageN(i);
            pageDict.put(PdfName.CROPBOX, rect);
        }
        PdfStamper stamper = new PdfStamper(reader, new FileOutputStream(output));
        stamper.close();
    }

    private static PdfImportedPage getPage(PdfWriter writer, PdfReader reader, int page) {
        if (page > 0 && page <= reader.getNumberOfPages())
            return writer.getImportedPage(reader, page);
        else
            return null;
    }

    private static void twoup(String input) throws Exception {
        String output = input.replace(".pdf", "-twoup.pdf");
        PdfReader reader = new PdfReader(input);
        int n = reader.getNumberOfPages();
        Rectangle pageSize = reader.getPageSize(1);

        System.out.println("Input page size: " + pageSize);
        Document doc = new Document(PageSize.A4.rotate(), 0, 0, 0, 0);
        PdfWriter writer = PdfWriter.getInstance(doc, new FileOutputStream(output));
        doc.open();
        // splitLine(doc, writer);
        PdfContentByte cb = writer.getDirectContent();
        float bottom = (doc.top() - pageSize.getHeight()) / 2 + kOffset;
        float left = doc.right() / 2 - (pageSize.getWidth() + kTextWidth) / 2 - kMargin;
        float right = doc.right() / 2 - (pageSize.getWidth() - kTextWidth) / 2 + kMargin;

        for (int i = 0; i <= n;) {
            PdfImportedPage page = getPage(writer, reader, i++);
            if (page != null)
                cb.addTemplate(page, left, bottom);

            page = getPage(writer, reader, i++);
            if (page != null)
                cb.addTemplate(page, right, bottom);

            doc.newPage();
        }
        doc.close();
    }

    private static void splitLine(Document doc, PdfWriter writer) {
        PdfContentByte cb = writer.getDirectContentUnder();
        cb.moveTo(doc.right() / 2, doc.bottom());
        cb.lineTo(doc.right() / 2, doc.top());
        cb.stroke();
    }

    private static void booklet(String input) throws Exception {
        String output = input.replace(".pdf", "-booklet.pdf");
        PdfReader reader = new PdfReader(input);
        int n = reader.getNumberOfPages();
        Rectangle pageSize = reader.getPageSize(1);

        System.out.println("Input page size: " + pageSize);
        Document doc = new Document(PageSize.A4.rotate(), 0, 0, 0, 0);
        PdfWriter writer = PdfWriter.getInstance(doc, new FileOutputStream(output));
        doc.open();
        splitLine(doc, writer);
        int[] pages = new int[(n + 3) / 4 * 4];
        int x = 1, y = pages.length;
        for (int i = 0; i < pages.length;) {
            pages[i++] = y--;
            pages[i++] = x++;
            pages[i++] = x++;
            pages[i++] = y--;
        }
        PdfContentByte cb = writer.getDirectContent();
        float bottom = (doc.top() - pageSize.getHeight()) / 2 + kOffset;
        float left = doc.right() / 2 - (pageSize.getWidth() + kTextWidth) / 2 - kMargin;
        float right = doc.right() / 2 - (pageSize.getWidth() - kTextWidth) / 2 + kMargin;

        for (int i = 0; i < pages.length;) {
            PdfImportedPage page = getPage(writer, reader, pages[i++]);
            if (page != null)
                cb.addTemplate(page, left, bottom);

            page = getPage(writer, reader, pages[i++]);
            if (page != null)
                cb.addTemplate(page, right, bottom);

            doc.newPage();
        }
        doc.close();
    }

    public static void main(String[] args) {
        if (args.length >= 2) {
            String cmd = args[0];
            String input = args[1];

            try {
                if (cmd.equals("crop")) {
                    crop(input);
                } else if (cmd.equals("twoup")) {
                    twoup(input);
                } else if (cmd.equals("booklet")) {
                    booklet(input);
                } else {
                    help();
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        } else {
            help();
        }
    }

}
