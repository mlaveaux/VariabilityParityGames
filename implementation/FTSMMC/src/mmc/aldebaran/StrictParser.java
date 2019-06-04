package mmc.aldebaran;

import mmc.models.Label;

public class StrictParser extends Parser {
    @Override
    protected int parseStartIndex() {
        int startIndex = super.parseStartIndex();
        if (startIndex != 0) {
            // http://cadp.inria.fr/man/aut.html
            //throw new StrictParseException("Initial state must be 0 according to specification", this.getIndex());
        }

        return startIndex;
    }

    @Override
    protected void parseHeader() {
        super.parseHeader();
        this.eatToken(TokenType.NEWLINE);
    }

    @Override
    protected void parseTransition() {
        super.parseTransition();
        this.eatToken(TokenType.NEWLINE);
    }

    @Override
    protected int parseNumber() {
        Token token = this.nextToken(TokenType.NUMBER);
        int number = Integer.parseInt(token.getData());

        if (number != 0 && token.getData().charAt(0) == '0') {
            throw new StrictParseException("Number has leading a leading 0", this.getIndex());
        }

        return number;
    }

    @Override
    protected Label parseLabel() {
        Label label = super.parseLabel();
        if (label.getText().length() > 5000) {
            throw new StrictParseException("A label can contain at most 5000 characters", this.getIndex());
        }

        return label;
    }
    @Override
    protected boolean isLayout(Token token) {
        return token.getType() == TokenType.LAYOUT;
    }
}
