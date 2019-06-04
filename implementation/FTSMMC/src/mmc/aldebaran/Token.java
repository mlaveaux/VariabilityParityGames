package mmc.aldebaran;

import java.util.Objects;

public class Token {
    private final TokenType type;
    private final String data;

    public Token(TokenType type, String data) {
        this.type = type;
        Objects.requireNonNull(data);
        this.data = data;
    }

    public TokenType getType() {
        return this.type;
    }

    public String getData() {
        return this.data;
    }

    @Override
    public String toString() {
        return "Token{" +
            "type=" + this.type +
            ", data='" + this.data + '\'' +
            '}';
    }
}
