package mmc.aldebaran;

import java.util.*;
import java.util.regex.Matcher;

public class Lexer implements Iterator<Token> {
    private int index;
    private CharSequence input;
    private final Map<TokenType, Matcher> matchers;
    private boolean eof;

    public Lexer(CharSequence input) {
        this.index = 0;
        Objects.requireNonNull(input);
        this.input = input;

        this.matchers = new EnumMap<>(TokenType.class);
        for (TokenType tokenType: TokenType.values()) {
            this.matchers.put(tokenType, tokenType.getPattern().matcher(this.input));
        }
        this.eof = false;
    }

    public int getIndex() {
        return this.index;
    }

    public static List<Token> lex(CharSequence input) {
        Lexer lexer = new Lexer(input);
        List<Token> tokens = new ArrayList<>();
        lexer.forEachRemaining(tokens::add);

        return tokens;
    }

    @Override
    public boolean hasNext() {
        return !this.eof;
    }

    @Override
    public Token next() throws LexException {
        if (!this.hasNext()) {
            throw new NoSuchElementException();
        }

        Token nextToken;
        if (this.input.length() > 0) {
            nextToken = findLongestPrefixMatch();
            this.advance(nextToken.getData().length());
        }
        else {
            this.eof = true;
            nextToken = new Token(TokenType.EOF, "");
        }

        return nextToken;
    }

    private Token findLongestPrefixMatch() throws LexException {
        Token nextToken = null;
        for (Map.Entry<TokenType, Matcher> entry : this.matchers.entrySet()) {
            TokenType tokenType = entry.getKey();
            Matcher matcher = entry.getValue();

            if (matcher.lookingAt()) {
                String data = matcher.group();
                if (nextToken == null || data.length() > nextToken.getData().length()) {
                    nextToken = new Token(tokenType, data);
                }
            }
        }

        if (nextToken == null) {
            throw new LexException(this.index);
        }

        return nextToken;
    }

    private void advance(int chars) {
        this.index += chars;
        this.input = this.input.subSequence(chars, this.input.length());
        this.matchers.forEach(
                (tokenType, matcher) -> matcher.region(matcher.regionStart() + chars, matcher.regionEnd()));
    }
}
