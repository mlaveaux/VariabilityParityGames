package mmc.aldebaran;

public class LexException extends SyntaxException {
    public LexException(int index) {
        super(String.format("Did not recognize token starting at index %s", index), index);
    }
}
