# syntax=docker/dockerfile:1

# Usage:
#     docker buildx build -t promkill:latest .
#     docker run --rm --publish 127.0.0.1:8080:8080 --name promkill promkill:latest
#     curl localhost:8080/metrics
#
#  or
#     kind load docker-image promkill:latest
#     kubectl apply --context kind-kind -f config/k8s/deployment.yaml


FROM golang:1.21-alpine AS build
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY *.go ./
RUN CGO_ENABLED=0 GOOS=linux go build -o /usr/local/bin/promkill

FROM scratch
COPY --from=build /usr/local/bin/promkill /usr/local/bin/promkill
EXPOSE 8080
CMD ["/usr/local/bin/promkill"]
