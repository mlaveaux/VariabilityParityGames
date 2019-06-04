package mmc.aldebaran;

public class ParseException extends SyntaxException {
    public ParseException(int index) {
        this(String.format("Unexpected token starting at index %s", index), index);
    }

    public ParseException(String message, int index) {
        super(message, index);
    }
}
