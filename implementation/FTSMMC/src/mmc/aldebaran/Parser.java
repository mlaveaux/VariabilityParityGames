package mmc.aldebaran;

import mmc.models.Label;
import mmc.models.Lts;
import mmc.models.State;

public class Parser {
    private Lexer lexer;
    private int transitions;
    private int states;
    private Lts lts;

    public Parser() {
        this.reset();
    }

    public Lts parse(Lexer lexer) {
        this.reset();
        this.lexer = lexer;

        this.parseHeader();
        for (int i = 0; i < this.transitions; i++) {
            this.parseTransition();
        }
        this.eatToken(TokenType.EOF);

        return this.lts;
    }

    protected void parseHeader() {
        this.eatToken(TokenType.DES);

        this.eatToken(TokenType.LPAREN);

        int startIndex = this.parseStartIndex();

        this.eatToken(TokenType.COMMA);

        this.transitions = this.parseNumber();

        this.eatToken(TokenType.COMMA);

        this.states = this.parseNumber();

        this.eatToken(TokenType.RPAREN);

        this.lts = new Lts(this.states, startIndex);
    }

    protected int parseStartIndex() {
        int startIndex = this.parseNumber();

        return startIndex;
    }

    protected int parseNumber() {
        Token token = this.nextToken(TokenType.NUMBER);
        int number = Integer.parseInt(token.getData());

        return number;
    }

    protected void parseTransition() {
        this.eatToken(TokenType.LPAREN);

        State start = this.parseState();

        this.eatToken(TokenType.COMMA);

        Label label = this.parseLabel();

        this.eatToken(TokenType.COMMA);

        State end = this.parseState();

        this.eatToken(TokenType.RPAREN);

        boolean success = start.addTransition(label, end);
        if (!success) {
            throw new ParseException("Duplicate transition encountered", this.getIndex());
        }
    }

    protected Label parseLabel() {
        Token token = this.nextToken();
        String labelText = token.getData();
        switch (token.getType()) {
            case DES:
                break;
            case STRING:
                if (labelText.charAt(0) == '"' && labelText.charAt(labelText.length() - 1) == '"') {
                    labelText = labelText.substring(1, labelText.length() - 1);
                }
                break;
            default:
                throw new ParseException(this.getIndex());

        }

        return new Label(labelText);
    }

    protected State parseState() {
        int number = this.parseNumber();
        if (number >= this.states) {
            throw new ParseException(String.format("Unknown state with number %s", number), this.getIndex());
        }

        return this.lts.getState(number);
    }

    protected boolean isLayout(Token token) {
        return token.getType() == TokenType.LAYOUT || token.getType() == TokenType.NEWLINE;
    }

    protected final Token nextToken() {
        Token token = this.lexer.next();
        while (this.isLayout(token)) {
            token = this.lexer.next();
        }

        return token;
    }

    protected final Token nextToken(TokenType tokenType) {
        Token token = this.nextToken();
        if (token.getType() != tokenType) {
            throw new ParseException(this.getIndex());
        }

        return token;
    }

    protected final void eatToken(TokenType tokenType) {
        this.nextToken(tokenType);
    }

    protected final int getIndex() {
        return this.lexer.getIndex();
    }

    protected void reset() {
        this.lexer = null;
        this.states = 0;
        this.transitions = 0;
        this.lts = null;
    }
}
