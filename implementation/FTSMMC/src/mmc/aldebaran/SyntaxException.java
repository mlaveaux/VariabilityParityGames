package mmc.aldebaran;

public class SyntaxException extends RuntimeException {
    private final int index;

    public SyntaxException(String message, int index) {
        super(message);
        this.index = index;
    }

    public int getIndex() {
        return this.index;
    }
}
