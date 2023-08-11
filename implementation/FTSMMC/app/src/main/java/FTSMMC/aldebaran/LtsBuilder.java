package mmc.aldebaran;

import mmc.models.Lts;

public class LtsBuilder {
    public Lts build(CharSequence input) {
        Lexer lexer = new Lexer(input);
        Parser parser = new Parser();
        Lts lts = parser.parse(lexer);

        return lts;
    }

    public Lts buildStrict(CharSequence input) {
        Lexer lexer = new Lexer(input);
        StrictParser parser = new StrictParser();
        Lts lts = parser.parse(lexer);

        return lts;
    }
}
